#ifndef _NETMQ_PINGCMD_H_
#define _NETMQ_PINGCMD_H_

#include "Cmd.h"

class PingCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	PingCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~PingCmd() = default;

private:
	struct AckData
	{
		Cmd::Type type;
		Cmd::ReasonCode reason;
	} ackdata;

	void ExecuteCmd() override final;
	void ExecuteAck() override final;
};

#endif
