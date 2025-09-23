#ifndef _NETMQ_PUBLISHCMD_H_
#define _NETMQ_PUBLISHCMD_H_

#include <cstddef>
#include <span>

#include "Cmd.h"

class PublishCmd : public Cmd
{
public:
	PublishCmd(const std::span<std::byte> topic, const std::span<std::byte> msg);
	virtual ~PublishCmd();

	void Process() override;

protected:
	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

#endif
