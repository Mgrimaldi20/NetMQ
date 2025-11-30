#ifndef _NETMQ_CONNECTCMD_H_
#define _NETMQ_CONNECTCMD_H_

#include <string>

#include "framework/Bitmask.h"

#include "Cmd.h"

class ConnectCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	ConnectCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~ConnectCmd() = default;

private:
	enum class Flags : uint16_t;

	struct AckData
	{
		Cmd::Type type;
		Cmd::ReasonCode reason;
	} ackdata;

	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	Flags flags;

	std::string clientid;
	std::span<std::byte> authtoken;
};

template<>
struct Bitmask::EnableBitmaskOperators<ConnectCmd::Flags> : std::true_type {};

enum class ConnectCmd::Flags : uint16_t
{
	ClientId = Bitmask::Bit<Flags, 0>(),
	AuthTkn = Bitmask::Bit<Flags, 1>()
};

#endif
