#include <stdexcept>
#include <algorithm>
#include <array>

#include "cmd/Cmd.h"
#include "CmdSystem.h"

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

std::unique_ptr<Cmd> CmdSystem::ParseNetCommand(const std::span<std::byte> incoming)
{
	size_t offset = 0;

	std::span<std::byte, CMD_HEADER_SIZE> header(incoming.subspan(offset, CMD_HEADER_SIZE));
	if (!std::equal(header.begin(), header.end(), CMD_HEADER.begin()))
	{
		log.Warn("Header does not match the expected value");
		return nullptr;
	}

	offset += header.size();

	const Cmd::Type type = static_cast<Cmd::Type>(std::to_integer<uint8_t>(incoming[offset++]));
	const std::span<std::byte> params = incoming.subspan(offset, (incoming.size() - offset));

	switch (type)
	{
		case Cmd::Type::Connect:
			return std::make_unique<ConnectCmd>();

		case Cmd::Type::Publish:
			return std::make_unique<PublishCmd>(params);

		case Cmd::Type::Subscribe:
			return std::make_unique<SubscribeCmd>(params);

		case Cmd::Type::Unsubscribe:
			return std::make_unique<UnsubscribeCmd>(params);

		case Cmd::Type::Disconnect:
			return std::make_unique<DisconnectCmd>();

		default:
			log.Warn("Unknown command type parsed");
			return nullptr;
	}
}
