#ifndef _NETMQ_CMDSYSTEM_H_
#define _NETMQ_CMDSYSTEM_H_

#include <cstddef>
#include <memory>
#include <span>

#include "cmd/Cmd.h"
#include "Log.h"

class CmdSystem
{
public:
	CmdSystem(Log &log);
	~CmdSystem();

	std::unique_ptr<Cmd> ParseNetCommand(const std::span<std::byte> incoming);

private:
	Log &log;
};

#endif
