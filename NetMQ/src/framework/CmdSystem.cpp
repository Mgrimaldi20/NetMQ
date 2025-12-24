#include "Cmd.h"
#include "mqtt/cmd/ConnectCmd.h"
#include "mqtt/cmd/PublishCmd.h"
#include "mqtt/cmd/SubscribeCmd.h"
#include "mqtt/cmd/UnsubscribeCmd.h"
#include "mqtt/cmd/PingReqCmd.h"
#include "mqtt/cmd/DisconnectCmd.h"
#include "mqtt/cmd/AuthCmd.h"

#include "CmdSystem.h"

CmdSystem::CmdSystem(Log &log)
	: log(log)
{
	log.Info("Command System started");
}

CmdSystem::~CmdSystem()
{
	log.Info("Shutting down the Command System");
}

std::unique_ptr<Cmd> CmdSystem::ParseCommand(std::shared_ptr<IOContext> ioctx, ByteBuffer &incoming)
{
	if (incoming.Size() < 1)
	{
		log.Warn("Received command with insufficient data to parse command type");
		return nullptr;
	}

	Cmd::Type cmd = static_cast<Cmd::Type>(incoming[0] & std::byte(0xF0));

	switch (cmd)
	{
		case Cmd::Type::Connect:
			return std::make_unique<ConnectCmd>(ConnectCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::Publish:
			return std::make_unique<PublishCmd>(PublishCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::Subscribe:
			return std::make_unique<SubscribeCmd>(SubscribeCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::Unsubscribe:
			return std::make_unique<UnsubscribeCmd>(UnsubscribeCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::PingReq:
			return std::make_unique<PingReqCmd>(PingReqCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::Disconnect:
			return std::make_unique<DisconnectCmd>(DisconnectCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::Auth:
			return std::make_unique<AuthCmd>(AuthCmd::Token {}, ioctx, manager, incoming);

		case Cmd::Type::Reserved:
		default:
			log.Warn("Unknown command type parsed");
			return nullptr;
	}
}
