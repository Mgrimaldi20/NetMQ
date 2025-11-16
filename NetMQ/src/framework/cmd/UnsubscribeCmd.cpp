#include <mutex>
#include <vector>

#include "UnsubscribeCmd.h"

UnsubscribeCmd::UnsubscribeCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	size_t offset = 0;

	std::pair<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

void UnsubscribeCmd::ExecuteCmd() const
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

void UnsubscribeCmd::ExecuteAck() const
{
}
