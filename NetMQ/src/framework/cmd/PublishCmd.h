#ifndef _NETMQ_PUBLISHCMD_H_
#define _NETMQ_PUBLISHCMD_H_

#include <cstddef>
#include <span>

#include "Cmd.h"

class PublishCmd : public Cmd
{
public:
	PublishCmd(const std::span<const std::byte> topic, const std::span<const std::byte> msg);
	virtual ~PublishCmd();

	void Process() override;

protected:
	const std::span<const std::byte> topic;
	const std::span<const std::byte> msg;
};

#endif
