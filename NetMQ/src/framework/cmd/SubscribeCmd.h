#ifndef _NETMQ_SUBSCRIBECMD_H_
#define _NETMQ_SUBSCRIBECMD_H_

#include "Cmd.h"

class SubscribeCmd : public Cmd
{
public:
	SubscribeCmd(std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~SubscribeCmd() = default;

protected:
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;

private:
	std::span<std::byte> topic;

	friend class CmdSystem;
};

#endif
