#ifndef _NETMQ_CONNECTCMD_H_
#define _NETMQ_CONNECTCMD_H_

#include <string>

#include "framework/Bitmask.h"

#include "Cmd.h"

class ConnectCmd : public Cmd
{
public:
	ConnectCmd(std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~ConnectCmd() = default;

protected:
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;

private:
	enum class Flags : uint16_t;

	Flags flags;

	std::string clientid;
	std::span<std::byte> authtoken;

	friend class CmdSystem;
};

template<>
struct Bitmask::EnableBitmaskOperators<ConnectCmd::Flags> : std::true_type {};

enum class ConnectCmd::Flags : uint16_t
{
	ClientId = Bitmask::Bit<Flags, 0>(),
	AuthTkn = Bitmask::Bit<Flags, 1>()
};

#endif
