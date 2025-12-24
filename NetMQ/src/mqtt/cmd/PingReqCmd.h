#ifndef _NETMQ_PINGCMD_H_
#define _NETMQ_PINGCMD_H_

#include "framework/ByteBuffer.h"
#include "framework/Cmd.h"

class PingReqCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	PingReqCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params);
	virtual ~PingReqCmd() = default;

private:
	struct AckData
	{
		Cmd::Type type;
		uint8_t remaininglen;
	} ackdata;

	void ExecuteCmd() override final;
	void ExecuteAck() override final;
};

#endif
