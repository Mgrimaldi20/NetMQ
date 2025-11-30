#include <type_traits>

#include "PingCmd.h"

PingCmd::PingCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager),
	ackdata({ .type = Cmd::Type::Ping })
{
	(void)params;
}

void PingCmd::ExecuteCmd()
{
	ioctx->SetClientID("PING_CLIENT");

	ackdata.reason = Cmd::ReasonCode::Success;
}

void PingCmd::ExecuteAck()
{
	ioctx->PostSend(
		ackbuilder
		.AppendUInt<Cmd::Type>(ackdata.type)
		.AppendUInt<Cmd::ReasonCode>(ackdata.reason)
		.Build()
	);

	ioctx->CloseClient();
}
