#include <stdexcept>
#include <algorithm>
#include <format>
#include <array>
#include <unordered_set>

#include "framework/UUID.h"

#include "ConnectCmd.h"

static std::unordered_set<std::string> usedidset;

ConnectCmd::ConnectCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params)
	: Cmd(ioctx, manager),
	ackdata({ .type = Cmd::Type::Connect })
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
		ackdata.reason = Cmd::ReasonCode::UnsupportedProtocolVersion;

	offset += header.size();

	std::pair<size_t, uint8_t> version = CmdUtil::ReadUInt<uint8_t>(params, offset);
	if (std::get<1>(version) != NETMQ_VERSION)
		ackdata.reason = Cmd::ReasonCode::UnsupportedProtocolVersion;

	offset += std::get<0>(version);

	std::pair<size_t, Flags> parsedflags = CmdUtil::ReadUInt<Flags>(params, offset);
	offset += std::get<0>(parsedflags);
	flags = std::get<1>(parsedflags);

	if (Bitmask::HasFlag(flags, Flags::ClientId))
	{
		std::pair<size_t, uint8_t> packetsize = CmdUtil::ReadUInt<uint8_t>(params, offset);
		offset += std::get<0>(packetsize);
		uint8_t clientidlen = std::get<1>(packetsize);
		if (clientidlen == 0)
			ackdata.reason = Cmd::ReasonCode::InvalidClientID;

		std::span<std::byte> tmpclientid = params.subspan(offset, clientidlen);
		std::transform(tmpclientid.begin(), tmpclientid.end(), std::back_inserter(clientid), [](std::byte b) { return static_cast<char>(b); });

		if (!usedidset.insert(clientid).second || !std::all_of(clientid.begin(), clientid.end(), [](unsigned char c) { return std::isprint(c); }))
			ackdata.reason = Cmd::ReasonCode::InvalidClientID;

		offset += clientidlen;
	}

	else	// no client ID provided, so generate one
	{
		static constexpr unsigned int MAX_CLIENTID_GENERATION_ATTEMPTS = 3;
		bool unique = false;

		for (unsigned int i=0; i<MAX_CLIENTID_GENERATION_ATTEMPTS && !unique; i++)
		{
			auto res = usedidset.insert(NetMQ::UUID::GenerateV4());

			unique = res.second;
			if (!unique)
			{
				clientid.clear();
				continue;
			}

			clientid = *res.first;
		}

		if (!unique)
			ackdata.reason = Cmd::ReasonCode::InvalidClientID;
	}

	if (Bitmask::HasFlag(flags, Flags::AuthTkn))
	{
		std::pair<size_t, uint8_t> tokenlen = CmdUtil::ReadUInt<uint8_t>(params, offset);
		offset += std::get<0>(tokenlen);
		uint8_t authtokenlen = std::get<1>(tokenlen);

		if (authtokenlen == 0)
			ackdata.reason = Cmd::ReasonCode::BadAuthenticationMethod;

		authtoken = params.subspan(offset, authtokenlen);
		offset += authtokenlen;
	}
}

void ConnectCmd::ExecuteCmd()
{
	if (ioctx->GetConnected().load())
		return;

	ioctx->SetClientID(clientid);
	ioctx->SetConnected(true);

	ackdata.reason = Cmd::ReasonCode::Success;
}

void ConnectCmd::ExecuteAck()
{
	ioctx->PostSend(
		ackbuilder
		.AppendUInt<Cmd::Type>(ackdata.type)
		.AppendUInt<Cmd::ReasonCode>(ackdata.reason)
		.Build()
	);
}
