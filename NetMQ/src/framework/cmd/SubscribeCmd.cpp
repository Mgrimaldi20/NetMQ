#include "SubscribeCmd.h"

SubscribeCmd::SubscribeCmd(const std::span<const std::byte> &topic)
	: topic(topic)
{
}

SubscribeCmd::~SubscribeCmd()
{
}

void SubscribeCmd::Process()
{
}
