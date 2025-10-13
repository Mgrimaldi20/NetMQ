#include "Cmd.h"

ConnectCmd::ConnectCmd(const std::span<std::byte> &params) noexcept
{
	(void)params;
}

PublishCmd::PublishCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
	offset += topiclen;

	ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t msglen = std::get<1>(ret);
	msg = params.subspan(offset, msglen);
}

SubscribeCmd::SubscribeCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

UnsubscribeCmd::UnsubscribeCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

DisconnectCmd::DisconnectCmd(const std::span<std::byte> &params) noexcept
{
	(void)params;
}
