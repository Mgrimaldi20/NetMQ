#ifndef _NETMQ_CMDSYSTEM_H_
#define _NETMQ_CMDSYSTEM_H_

#include <cstddef>
#include <memory>
#include <span>

#include "SubManager.h"
#include "Log.h"

#include "cmd/Cmd.h"

#include "sys/win32/io/IOContext.h"

class CmdSystem
{
public:
	CmdSystem(Log &log);
	~CmdSystem();

	std::unique_ptr<Cmd> ParseCommand(std::shared_ptr<IOContext> ioctx, std::span<std::byte> incoming);

private:
	SubManager manager;

	Log &log;
};

#endif
