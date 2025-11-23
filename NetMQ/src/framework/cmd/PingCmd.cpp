#include <type_traits>

#include "PingCmd.h"

PingCmd::PingCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager)
{
	(void)params;

	ackbuilder.AppendUInt<std::underlying_type_t<Cmd::Type>>(std::underlying_type_t<Cmd::Type>(Cmd::Type::Ping));
}

void PingCmd::ExecuteCmd()
{
	ioctx->SetClientID("PING_CLIENT");
	ackbuilder.AppendUInt<std::underlying_type_t<ReasonCode>>(std::underlying_type_t<ReasonCode>(ReasonCode::Success));
}

void PingCmd::ExecuteAck()
{
	ioctx->PostSend(ackbuilder.Build());
	ioctx->CloseClient();
}
