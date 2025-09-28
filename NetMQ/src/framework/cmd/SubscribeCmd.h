#ifndef _NETMQ_SUBSCRIBECMD_H_
#define _NETMQ_SUBSCRIBECMD_H_

#include <cstddef>
#include <span>

#include "Cmd.h"

class SubscribeCmd : public Cmd
{
public:
	SubscribeCmd(const std::span<const std::byte> &topic);
	virtual ~SubscribeCmd();

	void Process() override;

protected:
	const std::span<const std::byte> &topic;
};

#endif
