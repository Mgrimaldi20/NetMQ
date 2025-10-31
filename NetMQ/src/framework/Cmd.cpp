#include <stdexcept>
#include <format>
#include <array>

#include "sys/Cmd.h"

static constexpr uint8_t NETMQ_VERSION = 1;

ConnectCmd::ConnectCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	static constexpr std::array<std::byte, 5> HEADER_BYTES =
	{
		std::byte('N'),
		std::byte('E'),
		std::byte('T'),
		std::byte('M'),
		std::byte('Q')
	};

	size_t offset = 0;

	std::span<std::byte, HEADER_BYTES.size()> header(params.subspan(offset, HEADER_BYTES.size()));
	if (!std::equal(header.begin(), header.end(), HEADER_BYTES.begin()))
		throw std::runtime_error("Header does not match the expected value");

	offset += header.size();

	std::tuple<size_t, uint8_t> version = CmdUtil::ReadUInt<uint8_t>(params, offset);
	if (std::get<1>(version) != NETMQ_VERSION)
		throw std::runtime_error(std::format("Version parsed ({}) does not equal the implemented NetMQ protocol version of the server ({})", std::get<1>(version), NETMQ_VERSION));

	offset += std::get<0>(version);

	std::tuple<size_t, std::underlying_type_t<Flags>> parsedflags = CmdUtil::ReadUInt<std::underlying_type_t<Flags>>(params, offset);
	offset += std::get<0>(parsedflags);
	flags = static_cast<Flags>(std::get<1>(parsedflags));

	if (Bitmask::HasFlag(flags, Flags::ClientId))
	{
		std::tuple<size_t, uint8_t> packetsize = CmdUtil::ReadUInt<uint8_t>(params, offset);
		offset += std::get<0>(packetsize);
		uint8_t clientidlen = std::get<1>(packetsize);
		clientid = params.subspan(offset, clientidlen);
	}

	if (Bitmask::HasFlag(flags, Flags::AuthTkn))
	{
	}
}

PublishCmd::PublishCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
	offset += topiclen;

	ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t msglen = std::get<1>(ret);
	msg = params.subspan(offset, msglen);
}

SubscribeCmd::SubscribeCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	size_t offset = 0;

	std::tuple<size_t, std::underlying_type_t<Flags>> parsedflags = CmdUtil::ReadUInt<std::underlying_type_t<Flags>>(params, offset);
	offset += std::get<0>(parsedflags);
	flags = static_cast<Flags>(std::get<1>(parsedflags));

	if (Bitmask::HasFlag(flags, Flags::None))
		throw std::runtime_error("Invalid flags specified, flags must be set for Subscribe command");

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

UnsubscribeCmd::UnsubscribeCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

DisconnectCmd::DisconnectCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	(void)params;
}

PingCmd::PingCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params)
	: Cmd(ioctx)
{
	(void)params;
}
