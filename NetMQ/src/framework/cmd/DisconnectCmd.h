#ifndef _NETMQ_DISCONNECTCMD_H_
#define _NETMQ_DISCONNECTCMD_H_

#include "Cmd.h"

class DisconnectCmd : public Cmd
{
public:
	DisconnectCmd();
	virtual ~DisconnectCmd();

	void Process() override;
};

#endif
