#ifndef _NETMQ_SUBSCRIBECMD_H_
#define _NETMQ_SUBSCRIBECMD_H_

#include "Cmd.h"

class SubscribeCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	SubscribeCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~SubscribeCmd() = default;

private:
	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	std::span<std::byte> topic;
};

#endif
