#include <cstdint>
#include <algorithm>
#include <array>

#include "cmd/Cmd.h"
#include "cmd/ConnectCmd.h"
#include "cmd/PublishCmd.h"
#include "cmd/SubscribeCmd.h"
#include "cmd/UnsubscribeCmd.h"
#include "cmd/DisconnectCmd.h"
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

namespace
{
	inline size_t ReadU32BigEndian(const std::span<const std::byte> buffer, size_t offset, uint32_t &out)
	{
		if (buffer.size() - out < 4)
			return 0;

		out = (std::to_integer<uint32_t>(buffer[offset + 0]) << 24)
			| (std::to_integer<uint32_t>(buffer[offset + 1]) << 16)
			| (std::to_integer<uint32_t>(buffer[offset + 2]) << 8)
			| (std::to_integer<uint32_t>(buffer[offset + 3]));

		return 4;
	}
}

CmdSystem::CmdSystem(const Log &log)
	: log(log)
{
	log.Info("Command system started");
}

CmdSystem::~CmdSystem()
{
	log.Info("Shutting down the command system");
}

void CmdSystem::ExecuteCommand(const std::span<std::byte> incoming)
{
	size_t offset = 0;

	std::span<std::byte, CMD_HEADER_SIZE> header(incoming.subspan(offset, CMD_HEADER_SIZE));

	if (!std::equal(header.begin(), header.end(), CMD_HEADER.begin()))
	{
		log.Warn("Header does not match the expected value, dropping message");
		return;
	}

	offset += header.size();

	const Cmd::Type type = static_cast<Cmd::Type>(std::to_integer<uint8_t>(incoming[offset++]));

	switch (type)
	{
		case Cmd::Type::Connect:
		{
			ConnectCmd conn;
			conn();

			break;
		}

		case Cmd::Type::Publish:
		{
			uint32_t topiclen = 0;
			offset += ReadU32BigEndian(incoming, offset, topiclen);
			std::span<std::byte> topic(incoming.subspan(offset, topiclen));
			offset += topiclen;

			uint32_t msglen = 0;
			offset += ReadU32BigEndian(incoming, offset, msglen);
			std::span<std::byte> msg(incoming.subspan(offset, msglen));

			PublishCmd pub(topic, msg);
			pub();

			break;
		}

		case Cmd::Type::Subscribe:
		{
			uint32_t topiclen = 0;
			offset += ReadU32BigEndian(incoming, offset, topiclen);
			std::span<std::byte> topic(incoming.subspan(offset, topiclen));

			SubscribeCmd sub(topic);
			sub();

			break;
		}

		case Cmd::Type::Unsubscribe:
		{
			uint32_t topiclen = 0;
			offset += ReadU32BigEndian(incoming, offset, topiclen);
			std::span<std::byte> topic(incoming.subspan(offset, topiclen));

			UnsubscribeCmd unsub(topic);
			unsub();

			break;
		}

		case Cmd::Type::Disconnect:
		{
			DisconnectCmd disconn;
			disconn();

			break;
		}

		default:
			log.Warn("Unknown command type parsed");
	}
}
