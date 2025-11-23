#include <mutex>

#include "PublishCmd.h"

PublishCmd::PublishCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager)
{
	size_t offset = 0;

	std::pair<size_t, std::underlying_type_t<Options>> parsedflags = CmdUtil::ReadUInt<std::underlying_type_t<Options>>(params, offset);
	offset += std::get<0>(parsedflags);
	options = static_cast<Options>(std::get<1>(parsedflags));

	std::pair<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
	offset += topiclen;

	ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t msglen = std::get<1>(ret);
	msg = params.subspan(offset, msglen);
}

void PublishCmd::ExecuteCmd()
{
	if (!ioctx->GetConnected().load())
		return;

	std::scoped_lock lock(manager.subsmtx);

	std::span<std::byte> topickey(topic.begin(), topic.end());
	auto iter = manager.subscriptions.find(topickey);

	if (iter != manager.subscriptions.end())
	{
		for (auto subscriber : iter->second)
			subscriber->PostSend(msg);
	}
}

void PublishCmd::ExecuteAck()
{
}

const bool PublishCmd::AckRequired() const noexcept
{
	return (options == Options::Ack) ? true : false;
}
