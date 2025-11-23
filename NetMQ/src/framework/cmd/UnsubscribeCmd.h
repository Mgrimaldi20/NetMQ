#ifndef _NETMQ_UNSUBSCRIBECMD_H_
#define _NETMQ_UNSUBSCRIBECMD_H_

#include "Cmd.h"

class UnsubscribeCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	UnsubscribeCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~UnsubscribeCmd() = default;

private:
	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	std::span<std::byte> topic;
};

#endif
