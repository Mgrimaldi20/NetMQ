#include <cstddef>
#include <mutex>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "../Cmd.h"
#include "io/IOContext.h"

template <>
struct std::hash<std::vector<std::byte>>	// C++ template instantiation magic
{
	std::size_t operator()(const std::vector<byte> &v) const
	{
		std::size_t seed = v.size();
		for (std::byte b : v)
			seed ^= static_cast<std::size_t>(b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}
};

std::unordered_map<std::vector<std::byte>, std::vector<std::shared_ptr<IOContext>>> subscriptions;
std::mutex subsmtx;

void ConnectCmd::operator()() const
{
	if (ioctx->GetConnected().load())
		return;

	ioctx->SetConnected(true);
}

void PublishCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	std::vector<std::byte> topickey(topic.begin(), topic.end());
	auto iter = subscriptions.find(topickey);

	if (iter != subscriptions.end())
	{
		for (auto subscriber : iter->second)
		{
			subscriber->GetOutgoingBuffer().assign(msg.begin(), msg.end());
			subscriber->PostSend();
		}
	}
}

void SubscribeCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	std::vector<std::byte> topickey(topic.begin(), topic.end());
	auto iter = subscriptions.find(topickey);

	if (iter != subscriptions.end())
	{
		std::vector<std::shared_ptr<IOContext>> &sublist = iter->second;

		if (std::find(sublist.begin(), sublist.end(), ioctx) == sublist.end())
			sublist.push_back(ioctx);
	}

	else
	{
		std::vector<std::shared_ptr<IOContext>> newsublist;
		newsublist.push_back(ioctx);
		subscriptions[topickey] = std::move(newsublist);
	}
}

void UnsubscribeCmd::operator()() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(subsmtx);

	std::vector<std::byte> topickey(topic.begin(), topic.end());
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
