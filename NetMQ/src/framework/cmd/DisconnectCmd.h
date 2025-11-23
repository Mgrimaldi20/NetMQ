#ifndef _NETMQ_DISCONNECTCMD_H_
#define _NETMQ_DISCONNECTCMD_H_

#include "Cmd.h"

class DisconnectCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	DisconnectCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~DisconnectCmd() = default;

private:
	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	const bool AckRequired() const noexcept override final;
};

#endif
