#ifndef _NETMQ_CMDSYSTEM_H_
#define _NETMQ_CMDSYSTEM_H_

#include <cstddef>
#include <memory>
#include <span>

#include "SubManager.h"
#include "Log.h"
#include "Cmd.h"
#include "ByteBuffer.h"

#include "sys/win32/io/IOContext.h"

/*
* Class: CmdSystem
* Handles the parsing and the creation of command objects from incoming data packets.
* Each command object encapsulates the logic for executing specific commands and acts upon a context.
* 
*	ParseCommand: Parses incoming data to identify the command type and creates the corresponding command object
*/
class CmdSystem
{
public:
	CmdSystem(Log &log);
	~CmdSystem();

	std::unique_ptr<Cmd> ParseCommand(std::shared_ptr<IOContext> ioctx, ByteBuffer &incoming);

private:
	SubManager manager;

	Log &log;
};

#endif
