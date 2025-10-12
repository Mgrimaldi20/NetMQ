#include "Cmd.h"

size_t CmdUtil::ReadU32BigEndian(const std::span<const std::byte> &buffer, const size_t offset, uint32_t &out) noexcept
{
	if ((buffer.size() - out) < 4)
		return 0;

	out = (std::to_integer<uint32_t>(buffer[offset + 0]) << 24)
		| (std::to_integer<uint32_t>(buffer[offset + 1]) << 16)
		| (std::to_integer<uint32_t>(buffer[offset + 2]) << 8)
		| (std::to_integer<uint32_t>(buffer[offset + 3]));

	return sizeof(uint32_t);
}

ConnectCmd::ConnectCmd(const std::span<std::byte> &params) noexcept
{
	(void)params;
}

PublishCmd::PublishCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	uint32_t topiclen = 0;
	offset += CmdUtil::ReadU32BigEndian(params, offset, topiclen);
	topic = params.subspan(offset, topiclen);
	offset += topiclen;

	uint32_t msglen = 0;
	offset += CmdUtil::ReadU32BigEndian(params, offset, msglen);
	msg = params.subspan(offset, msglen);
}

SubscribeCmd::SubscribeCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	uint32_t topiclen = 0;
	offset += CmdUtil::ReadU32BigEndian(params, offset, topiclen);
	topic = params.subspan(offset, topiclen);
}

UnsubscribeCmd::UnsubscribeCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	uint32_t topiclen = 0;
	offset += CmdUtil::ReadU32BigEndian(params, offset, topiclen);
	topic = params.subspan(offset, topiclen);
}

DisconnectCmd::DisconnectCmd(const std::span<std::byte> &params) noexcept
{
	(void)params;
}
