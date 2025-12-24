#ifndef _NETMQ_DISCONNECTCMD_H_
#define _NETMQ_DISCONNECTCMD_H_

#include "framework/ByteBuffer.h"
#include "framework/Cmd.h"

class DisconnectCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	DisconnectCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params);
	virtual ~DisconnectCmd() = default;

private:
	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	const bool AckRequired() const noexcept override final;
};

#endif
