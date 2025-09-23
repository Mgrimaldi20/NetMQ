#include "PublishCmd.h"

PublishCmd::PublishCmd(const std::span<std::byte> topic, const std::span<std::byte> msg)
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
