#include "PingCmd.h"

PingCmd::PingCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager)
{
	(void)params;
}

void PingCmd::ExecuteCmd() const
{
}

void PingCmd::ExecuteAck() const
{
}
