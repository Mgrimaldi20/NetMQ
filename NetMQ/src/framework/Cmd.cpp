#include <iostream>

#include "Cmd.h"

/*
* Cmd
*/
void Cmd::operator()(const std::any &userdata, const CmdArgs &args) const
{
	func(userdata, args);
}

Cmd::Cmd(const std::string &name, const CmdFunction &cmdfunc, const std::string &description)
	: name(name),
	func(cmdfunc),
	description(description)
{
}

const std::string &Cmd::GetName()
{
	return name;
}

const std::string &Cmd::GetDescription()
{
	return description;
}

/*
* CmdSystem
*/
CmdSystem::CmdSystem(Log &log)
	: log(log),
	cmdmap()
{
}

CmdSystem::~CmdSystem()
{
	log.Info("Shutting down command system");
}

const Cmd &CmdSystem::RegisterCommand(Cmd &cmd)
{
	return cmdmap.insert_or_assign(cmd.GetName(), cmd).first->second;
}

const Cmd &CmdSystem::RegisterCommand(const std::string &name, const Cmd::CmdFunction &cmdfunc, const std::string &description)
{
	return cmdmap.insert_or_assign(name, Cmd(name, cmdfunc, description)).first->second;
}

const Cmd &CmdSystem::FindCommand(const std::string &name)
{
	return cmdmap.at(name);
}

void CmdSystem::ExecuteCommand(const std::any &userdata, const std::string &cmd)
{
	CmdArgs args(cmd);		// tokenizes the whole cmd string (args[0] is the cmd name)

	const std::string &func = args[0];

	if (cmdmap.contains(func))
		cmdmap.at(func)(userdata, args);
}
