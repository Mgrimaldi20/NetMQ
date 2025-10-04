#include "Cmd.h"

UnsubscribeCmd::UnsubscribeCmd(const std::span<std::byte> &params) noexcept
{
	size_t offset = 0;

	uint32_t topiclen = 0;
	offset += CmdUtil::ReadU32BigEndian(params, offset, topiclen);
	topic = params.subspan(offset, topiclen);
}

void UnsubscribeCmd::operator()() const
{
}
