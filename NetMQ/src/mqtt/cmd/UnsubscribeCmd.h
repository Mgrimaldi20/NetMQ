#ifndef _NETMQ_UNSUBSCRIBECMD_H_
#define _NETMQ_UNSUBSCRIBECMD_H_

#include "framework/ByteBuffer.h"
#include "framework/Cmd.h"

class UnsubscribeCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	UnsubscribeCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params);
	virtual ~UnsubscribeCmd() = default;

private:
	struct AckData
	{
		Cmd::Type type;
		Cmd::ReasonCode reason;
	} ackdata;

	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	std::span<std::byte> topic;
};

#endif
