#ifndef _NETMQ_UNSUBSCRIBECMD_H_
#define _NETMQ_UNSUBSCRIBECMD_H_

#include <cstddef>
#include <span>

#include "Cmd.h"

class UnsubscribeCmd : public Cmd
{
public:
	UnsubscribeCmd(const std::span<const std::byte> &topic);
	virtual ~UnsubscribeCmd();

	void Process() override;

protected:
	const std::span<const std::byte> &topic;
};

#endif
