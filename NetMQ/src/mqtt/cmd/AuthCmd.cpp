#include "AuthCmd.h"

AuthCmd::AuthCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params)
	: Cmd(ioctx, manager)
{
	(void)params;
}

void AuthCmd::ExecuteCmd()
{
	if (!ioctx->GetConnected().load())
		return;
}

void AuthCmd::ExecuteAck()
{
}

const bool AuthCmd::AckRequired() const noexcept
{
	return false;
}
