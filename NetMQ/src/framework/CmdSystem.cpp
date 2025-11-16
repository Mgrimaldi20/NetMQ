#include "cmd/PingCmd.h"
#include "cmd/ConnectCmd.h"
#include "cmd/PublishCmd.h"
#include "cmd/SubscribeCmd.h"
#include "cmd/UnsubscribeCmd.h"
#include "cmd/DisconnectCmd.h"

#include "CmdSystem.h"

CmdSystem::CmdSystem(Log &log)
	: log(log)
{
	log.Info("Command system started");
}

CmdSystem::~CmdSystem()
{
	log.Info("Shutting down the command system");
}

std::unique_ptr<Cmd> CmdSystem::ParseCommand(std::shared_ptr<IOContext> ioctx, std::span<std::byte> incoming) const
{
	size_t offset = 0;

	std::pair<size_t, uint8_t> cmd = CmdUtil::ReadUInt<uint8_t>(incoming, offset);
	offset += std::get<0>(cmd);

	std::span<std::byte> params = incoming.subspan(offset, (incoming.size() - offset));

	const Cmd::Type type = static_cast<Cmd::Type>(std::get<1>(cmd));

	switch (type)
	{
		case Cmd::Type::Ping:
			return std::make_unique<PingCmd>(ioctx, params);

		case Cmd::Type::Connect:
			return std::make_unique<ConnectCmd>(ioctx, params);

		case Cmd::Type::Publish:
			return std::make_unique<PublishCmd>(ioctx, params);

		case Cmd::Type::Subscribe:
			return std::make_unique<SubscribeCmd>(ioctx, params);

		case Cmd::Type::Unsubscribe:
			return std::make_unique<UnsubscribeCmd>(ioctx, params);

		case Cmd::Type::Disconnect:
			return std::make_unique<DisconnectCmd>(ioctx, params);

		default:
			log.Warn("Unknown command type parsed");
			return nullptr;
	}
}
