#include "PublishCmd.h"

PublishCmd::PublishCmd(const std::span<const std::byte> &topic, const std::span<const std::byte> &msg)
	: topic(topic),
	msg(msg)
{
}

PublishCmd::~PublishCmd()
{
}

void PublishCmd::Process()
{
}
