#ifndef _NETMQ_PUBLISHCMD_H_
#define _NETMQ_PUBLISHCMD_H_

#include "framework/ByteBuffer.h"
#include "framework/Cmd.h"

class PublishCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	PublishCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, ByteBuffer &params);
	virtual ~PublishCmd() = default;

private:
	struct AckData
	{
		Cmd::Type type;
		Cmd::ReasonCode reason;
	} ackdata;

	enum class Options : uint16_t
	{
		Ack,
		NoAck
	} options;

	void ExecuteCmd() override final;
	void ExecuteAck() override final;

	const bool AckRequired() const noexcept override final;

	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

#endif
