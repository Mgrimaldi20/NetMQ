#include <algorithm>
#include <array>

#include "cmd/Cmd.h"
#include "CmdSystem.h"

#include "sys/SysCmd.h"

constexpr size_t CMD_HEADER_SIZE = 5;

constexpr std::array<std::byte, CMD_HEADER_SIZE> CMD_HEADER =
{
	std::byte('N'),
	std::byte('E'),
	std::byte('T'),
	std::byte('M'),
	std::byte('Q')
};

CmdSystem::CmdSystem(Log &log)
	: log(log)
{
	log.Info("Command system started");
}

CmdSystem::~CmdSystem()
{
	log.Info("Shutting down the command system");
}

std::unique_ptr<Cmd> CmdSystem::ParseCommand(const std::span<std::byte> incoming) const
{
	size_t offset = 0;

	std::span<std::byte, CMD_HEADER_SIZE> header(incoming.subspan(offset, CMD_HEADER_SIZE));
	if (!std::equal(header.begin(), header.end(), CMD_HEADER.begin()))
	{
		log.Warn("Header does not match the expected value");
		return nullptr;
	}

	offset += header.size();

	uint32_t cmdnum = 0;
	offset += CmdUtil::ReadU32BigEndian(incoming, offset, cmdnum);
	const Cmd::Type type = static_cast<Cmd::Type>(cmdnum);

	const std::span<std::byte> params = incoming.subspan(offset, (incoming.size() - offset));

	switch (type)
	{
		case Cmd::Type::Connect:
			return std::make_unique<ConnectSysCmd>(params);

		case Cmd::Type::Publish:
			return std::make_unique<PublishSysCmd>(params);

		case Cmd::Type::Subscribe:
			return std::make_unique<SubscribeSysCmd>(params);

		case Cmd::Type::Unsubscribe:
			return std::make_unique<UnsubscribeSysCmd>(params);

		case Cmd::Type::Disconnect:
			return std::make_unique<DisconnectSysCmd>(params);

		default:
			log.Warn("Unknown command type parsed");
			return nullptr;
	}
}
