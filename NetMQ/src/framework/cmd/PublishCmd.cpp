#include "Cmd.h"

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

void PublishCmd::operator()() const
{
}
