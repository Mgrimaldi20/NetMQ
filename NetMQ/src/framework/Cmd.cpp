#include <iostream>

#include "Cmd.h"

Cmd::Cmd(Cmd::maptype_t &cmdmap)
	: cmdmap(cmdmap)
{
}

Cmd::~Cmd()
{
	std::cout << "Shutting down command system" << std::endl;
}

void Cmd::RegisterCommand(const std::string &name, const Cmd::CmdFunction_t &func)
{
	cmdmap.insert_or_assign(name, func);
}

void Cmd::BufferCommand(const std::string &cmd)
{
	cmdqueue.push(cmd);
}

void Cmd::ExecuteCommandBuffer()
{
	while (!cmdqueue.empty())
	{
		const std::string &cmd = cmdqueue.front();

		CmdArgs args(cmd);

		if (cmdmap.contains(args[0]))
			cmdmap.at(args[0])(args);	// tokenizes the whole cmd string, then calls the function with the args (args[0] is the cmd name)

		cmdqueue.pop();
	}
}
