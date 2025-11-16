#ifndef _NETMQ_UNSUBSCRIBECMD_H_
#define _NETMQ_UNSUBSCRIBECMD_H_

#include "Cmd.h"

class UnsubscribeCmd : public Cmd
{
public:
	UnsubscribeCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~UnsubscribeCmd() = default;

protected:
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;

private:
	std::span<std::byte> topic;
};

#endif
