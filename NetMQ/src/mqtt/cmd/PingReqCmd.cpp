#include <type_traits>

#include "PingReqCmd.h"

PingReqCmd::PingReqCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params)
	: Cmd(ioctx, manager),
	ackdata({ .type = Cmd::Type::PingResp, .remaininglen = 0 })
{
	(void)params;
}

void PingReqCmd::ExecuteCmd()
{
	if (!ioctx->GetConnected().load())
		return;
}

void PingReqCmd::ExecuteAck()
{
	ioctx->PostSend(
		ackbuilder
		.AppendUInt<Cmd::Type>(ackdata.type)
		.AppendUInt<uint8_t>(ackdata.remaininglen)
		.Build()
	);
}
