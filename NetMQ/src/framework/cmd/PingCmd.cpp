#include "PingCmd.h"

PingCmd::PingCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	(void)params;
}

void PingCmd::ExecuteCmd() const
{
}

void PingCmd::ExecuteAck() const
{
}
