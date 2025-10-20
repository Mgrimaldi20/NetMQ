#ifndef _NETMQ_CMDSYSTEM_H_
#define _NETMQ_CMDSYSTEM_H_

#include <cstddef>
#include <memory>
#include <span>

#include "sys/Cmd.h"

#include "Log.h"

class CmdSystem
{
public:
	CmdSystem(Log &log);
	~CmdSystem();

	std::unique_ptr<Cmd> ParseCommand(std::shared_ptr<IOContext> ioctx, const std::span<std::byte> incoming) const;

private:
	Log &log;
};

#endif
