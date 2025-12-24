#ifndef _NETMQ_CONNECTCMD_H_
#define _NETMQ_CONNECTCMD_H_

#include <string>

#include "framework/Bitmask.h"
#include "framework/ByteBuffer.h"
#include "framework/Cmd.h"

class ConnectCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	ConnectCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params);
	virtual ~ConnectCmd() = default;

private:
	enum class Flags : std::underlying_type_t<std::byte>;

	struct AckData
	{
		Cmd::Type type;
		Cmd::ReasonCode reason;
	} ackdata;

	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	std::string clientid;
};

template<>
struct Bitmask::EnableBitmaskOperators<ConnectCmd::Flags> : std::true_type {};

enum class ConnectCmd::Flags : std::underlying_type_t<std::byte>
{
	Reserved = Bitmask::Bit<Flags, 0>(),
	CleanStart = Bitmask::Bit<Flags, 1>(),
	WillFlag = Bitmask::Bit<Flags, 2>(),
	WillQoS1 = Bitmask::Bit<Flags, 3>(),
	WillQoS2 = Bitmask::Bit<Flags, 4>(),
	WillRetain = Bitmask::Bit<Flags, 5>(),
	PasswordFlag = Bitmask::Bit<Flags, 6>(),
	UsernameFlag = Bitmask::Bit<Flags, 7>()
};

#endif
