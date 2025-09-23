#include "cmd/Cmd.h"
#include "cmd/PublishCmd.h"
#include "CmdSystem.h"

CmdSystem::CmdSystem(const Log &log)
	: log(log)
{
	log.Info("Starting up the command system");
}

CmdSystem::~CmdSystem()
{
	log.Info("Shutting down the command system");
}

void CmdSystem::ExecuteCommand(std::span<std::byte> incoming)
{
	size_t offset = 0;

	std::span<std::byte, CMD_HEADER_SIZE> header(incoming.subspan(offset, CMD_HEADER_SIZE));
	offset += header.size();

	Cmd::Type type = static_cast<Cmd::Type>(incoming[offset]);
	offset++;

	switch (type)
	{
		case Cmd::Type::Connect:
		{
			/*log.Info("Clinet: {} connecting with configuration: {}");

			ConnectCmd conn(config);
			conn();

			log.Info("Client: {} connected successfully");*/

			break;
		}

		case Cmd::Type::Publish:
		{
			size_t topiclen = 0;
			std::span<std::byte> topic(incoming.subspan(offset, offset + topiclen));
			offset += topic.size();

			size_t msglen = 0;
			std::span<std::byte> msg(incoming.subspan(offset, offset + msglen));

			PublishCmd pub(topic, msg);
			pub();

			break;
		}

		case Cmd::Type::Subscribe:
		{
			size_t topiclen = 0;
			std::span<std::byte> topic(incoming.subspan(offset, offset + topiclen));

			/*SubscribeCmd sub(topic);
			sub();

			log.Info("Client: {} has subscribed to topic: {}");*/

			break;
		}

		case Cmd::Type::Unsubscribe:
		{
			size_t topiclen = 0;
			std::span<std::byte> topic(incoming.subspan(offset, offset + topiclen));

			/*UnsubscribeCmd unsub(topic);
			unsub();

			log.Info("Client: {} has unsubscribed from topic: {}");*/

			break;
		}

		case Cmd::Type::Disconnect:
		{
			/*DisconnectCmd disconn;
			disconn();

			log.Info("Client: {} has disconnected");*/

			break;
		}

		default:
			log.Warn("Unknown command type parsed");
	}
}
