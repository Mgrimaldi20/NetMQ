#ifndef _NETMQ_PUBLISHCMD_H_
#define _NETMQ_PUBLISHCMD_H_

#include "Cmd.h"

class PublishCmd : public Cmd
{
public:
	PublishCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~PublishCmd() = default;

protected:
	void ExecuteCmd() const override final;
	void ExecuteAck() const override final;

	const bool AckRequired() const noexcept override final;

private:
	enum class Options : uint16_t
	{
		Ack,
		NoAck
	} options;

	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

#endif
