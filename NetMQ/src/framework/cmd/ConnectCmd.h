#ifndef _NETMQ_CONNECTCMD_H_
#define _NETMQ_CONNECTCMD_H_

#include "Cmd.h"

class ConnectCmd : public Cmd
{
public:
	ConnectCmd();
	virtual ~ConnectCmd();

	void Process() override;
};

#endif
