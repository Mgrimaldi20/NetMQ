#include <mutex>
#include <vector>

#include "SubscribeCmd.h"

SubscribeCmd::SubscribeCmd(std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager)
{
	size_t offset = 0;

	std::pair<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

void SubscribeCmd::ExecuteCmd() const
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(manager.subsmtx);

	std::span<std::byte> topickey(topic.begin(), topic.end());
	auto iter = manager.subscriptions.find(topickey);

	if (iter != manager.subscriptions.end())
	{
		std::vector<std::shared_ptr<IOContext>> &sublist = iter->second;

		if (std::find(sublist.begin(), sublist.end(), ioctx) == sublist.end())
			sublist.push_back(ioctx);
	}

	else
	{
		std::vector<std::shared_ptr<IOContext>> newsublist({ ioctx });
		newsublist.push_back(ioctx);
		manager.subscriptions[std::move(std::vector<std::byte>(topickey.begin(), topickey.end()))] = std::move(newsublist);
	}
}

void SubscribeCmd::ExecuteAck() const
{
}
