#ifndef _NETMQ_CMDSYSTEM_H_
#define _NETMQ_CMDSYSTEM_H_

#include <cstddef>
#include <span>

#include "Log.h"

class CmdSystem
{
public:
	CmdSystem(const Log &log);
	~CmdSystem();

	void ExecuteCommand(const std::span<std::byte> incoming);

private:
	const Log &log;
};

#endif
