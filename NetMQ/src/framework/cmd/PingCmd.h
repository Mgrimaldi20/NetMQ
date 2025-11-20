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
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;
};

#endif
