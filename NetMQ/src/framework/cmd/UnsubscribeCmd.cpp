#include "UnsubscribeCmd.h"

UnsubscribeCmd::UnsubscribeCmd(const std::span<const std::byte> &topic)
	: topic(topic)
{
}

UnsubscribeCmd::~UnsubscribeCmd()
{
}

void UnsubscribeCmd::Process()
{
}
