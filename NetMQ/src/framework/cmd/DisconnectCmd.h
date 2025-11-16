#ifndef _NETMQ_DISCONNECTCMD_H_
#define _NETMQ_DISCONNECTCMD_H_

#include "Cmd.h"

class DisconnectCmd : public Cmd
{
public:
	DisconnectCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~DisconnectCmd() = default;

protected:
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;

	const bool AckRequired() const noexcept override final;
};

#endif
