#ifndef _NETMQ_PINGCMD_H_
#define _NETMQ_PINGCMD_H_

#include "Cmd.h"

class PingCmd : public Cmd
{
public:
	PingCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~PingCmd() = default;

protected:
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;
};

#endif
