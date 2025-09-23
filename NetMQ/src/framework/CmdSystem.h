#ifndef _NETMQ_CMDSYSTEM_H_
#define _NETMQ_CMDSYSTEM_H_

#include <cstddef>
#include <span>

#include "Log.h"

constexpr size_t CMD_HEADER_SIZE = 5;

class CmdSystem
{
public:
	CmdSystem(const Log &log);
	~CmdSystem();

	void ExecuteCommand(std::span<std::byte> incoming);

private:
	const Log &log;
};

#endif
