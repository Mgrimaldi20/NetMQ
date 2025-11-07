#include <cstddef>
#include <mutex>
#include <memory>
#include <functional>
#include <unordered_map>
#include <span>
#include <vector>

#include "../Cmd.h"
#include "io/IOContext.h"

struct ByteHash
{
	using is_transparent = void;

	std::size_t operator()(const std::vector<std::byte> &v) const
	{
		return HashBytes(std::span<const std::byte>(v));
	}

	std::size_t operator()(std::span<std::byte> s) const
	{
		return HashBytes(s);
	}

private:
	std::size_t HashBytes(std::span<const std::byte> bytes) const
	{
		std::size_t seed = bytes.size();
		for (std::byte b : bytes)
			seed ^= static_cast<std::size_t>(b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}
};

struct ByteEquality
{
	using is_transparent = void;

	bool operator()(const std::vector<std::byte> &lhs, const std::vector<std::byte> &rhs) const
	{
		return lhs == rhs;
	}

	bool operator()(const std::vector<std::byte> &lhs, std::span<const std::byte> rhs) const
	{
		return std::span<const std::byte>(lhs).size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	bool operator()(std::span<const std::byte> lhs, const std::vector<std::byte> &rhs) const
	{
		return operator()(rhs, lhs);
	}
};

static std::unordered_map<std::vector<std::byte>, std::vector<std::shared_ptr<IOContext>>, ByteHash, ByteEquality> subscriptions;
static std::mutex subsmtx;

void ConnectCmd::operator()() const
{
	if (ioctx->GetConnected().load())
		return;

	ioctx->SetClientID(clientid);
	ioctx->SetConnected(true);
}

void PublishCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	std::span<std::byte> topickey(topic.begin(), topic.end());
	auto iter = subscriptions.find(topickey);

	if (iter != subscriptions.end())
	{
		for (auto subscriber : iter->second)
			subscriber->PostSend(msg);
	}
}

void SubscribeCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	std::span<std::byte> topickey(topic.begin(), topic.end());
	auto iter = subscriptions.find(topickey);

	if (iter != subscriptions.end())
	{
		std::vector<std::shared_ptr<IOContext>> &sublist = iter->second;

		if (std::find(sublist.begin(), sublist.end(), ioctx) == sublist.end())
			sublist.push_back(ioctx);
	}

	else
	{
		std::vector<std::shared_ptr<IOContext>> newsublist({ ioctx });
		newsublist.push_back(ioctx);
		subscriptions[std::move(std::vector<std::byte>(topickey.begin(), topickey.end()))] = std::move(newsublist);
	}
}

void UnsubscribeCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	std::span<std::byte> topickey(topic.begin(), topic.end());
	auto iter = subscriptions.find(topickey);

	if (iter != subscriptions.end())
	{
		std::vector<std::shared_ptr<IOContext>> &sublist = iter->second;
		sublist.erase(std::remove(sublist.begin(), sublist.end(), ioctx), sublist.end());

		if (sublist.empty())
			subscriptions.erase(iter);
	}
}

void DisconnectCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	for (auto &[topic, sublist] : subscriptions)
		sublist.erase(std::remove(sublist.begin(), sublist.end(), ioctx), sublist.end());

	std::erase_if(subscriptions, [](const auto &pair) { return pair.second.empty(); });

	ioctx->CloseClient();
}

void PingCmd::operator()() const
{
}
