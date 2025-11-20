#ifndef _NETMQ_PUBLISHCMD_H_
#define _NETMQ_PUBLISHCMD_H_

#include "Cmd.h"

class PublishCmd : public Cmd
{
	friend class CmdSystem;

private:
	struct Token {};

public:
	PublishCmd(Token, std::shared_ptr<IOContext> ioctx, SubManager &manager, std::span<std::byte> params);
	virtual ~PublishCmd() = default;

private:
	enum class Options : uint16_t
	{
		Ack,
		NoAck
	} options;

	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;

	const bool AckRequired() const noexcept override final;

	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

#endif
