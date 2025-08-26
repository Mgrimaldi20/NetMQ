#include <iostream>

#include "Cmd.h"

Cmd::Cmd()
	: cmdmap()
{
}

Cmd::~Cmd()
{
	std::cout << "Shutting down command system" << std::endl;
}

void Cmd::RegisterCommand(const std::string &name, const Cmd::CmdFunction &func)
{
	cmdmap.insert_or_assign(name, func);
}

void Cmd::ExecuteCommand(const std::any &userdata, const std::string &cmd)
{
	CmdArgs args(cmd);		// tokenizes the whole cmd string (args[0] is the cmd name)

	const std::string &func = args[0];

	if (cmdmap.contains(func))
		cmdmap.at(func)(userdata, args);
}
