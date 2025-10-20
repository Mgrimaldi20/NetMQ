#include <stdexcept>
#include <format>
#include <array>

#include "sys/Cmd.h"

static constexpr uint8_t NETMQ_VERSION = 1;

ConnectCmd::ConnectCmd(std::shared_ptr<IOContext> ioctx, const std::span<std::byte> &params)
	: Cmd(ioctx)
{
	static constexpr size_t CMD_HEADER_SIZE = 5;

	static constexpr std::array<std::byte, CMD_HEADER_SIZE> CMD_HEADER =
	{
		std::byte('N'),
		std::byte('E'),
		std::byte('T'),
		std::byte('M'),
		std::byte('Q')
	};

	size_t offset = 0;

	std::span<std::byte, CMD_HEADER_SIZE> header(params.subspan(offset, CMD_HEADER_SIZE));
	if (!std::equal(header.begin(), header.end(), CMD_HEADER.begin()))
		throw std::runtime_error("Header does not match the expected value");

	offset += header.size();

	std::tuple<size_t, uint8_t> version = CmdUtil::ReadUInt<uint8_t>(params, offset);
	if (std::get<1>(version) != NETMQ_VERSION)
		throw std::runtime_error(std::format("Version parsed ({}) does not equal the implemented NetMQ protocol version of the server ({})", std::get<1>(version), NETMQ_VERSION));

	offset += std::get<0>(version);
}

PublishCmd::PublishCmd(std::shared_ptr<IOContext> ioctx, const std::span<std::byte> &params) noexcept
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

SubscribeCmd::SubscribeCmd(std::shared_ptr<IOContext> ioctx, const std::span<std::byte> &params) noexcept
	: Cmd(ioctx)
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

UnsubscribeCmd::UnsubscribeCmd(std::shared_ptr<IOContext> ioctx, const std::span<std::byte> &params) noexcept
	: Cmd(ioctx)
{
	size_t offset = 0;

	std::tuple<size_t, uint32_t> ret = CmdUtil::ReadUInt<uint32_t>(params, offset);
	offset += std::get<0>(ret);
	uint32_t topiclen = std::get<1>(ret);
	topic = params.subspan(offset, topiclen);
}

DisconnectCmd::DisconnectCmd(std::shared_ptr<IOContext> ioctx, const std::span<std::byte> &params) noexcept
	: Cmd(ioctx)
{
	(void)params;
}
