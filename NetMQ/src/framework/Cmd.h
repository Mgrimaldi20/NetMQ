#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <string>
#include <any>
#include <functional>
#include <unordered_map>

#include "Log.h"
#include "CmdArgs.h"

/*
* Class: Cmd
* This class is the representation of a command used by the system.
* These commands will be stored by a command system, or can be created free standing.
* 
*	operator(): Calls the command function with the supplied args
*/
class Cmd
{
public:
	using CmdFunction = std::function<void(const std::any &, const CmdArgs &)>;

	void operator()(const std::any &userdata, const CmdArgs &args) const;

	Cmd(const std::string &name, const CmdFunction &cmdfunc, const std::string &description);
	~Cmd() {};

	const std::string &GetName() const;
	const std::string &GetDescription() const;

private:
	std::string name;
	std::string description;
	CmdFunction func;
};

/*
* Class: CmdSystem
* The command system, responsible for registering any commands that the clients may call.
*
*	RegisterCommand: Registers a new command with the system, or modifies an existing one, push or emplace
*	FindCommand: Returns the command found from the specified name if it exists
*	ExecuteCommand: Tokenizes and executes the command string supplied, IOContext as userdata
*/
class CmdSystem
{
public:
	CmdSystem(Log &log);
	~CmdSystem();

	const Cmd &RegisterCommand(const Cmd &cmd);
	const Cmd &RegisterCommand(const std::string &name, const Cmd::CmdFunction &cmdfunc, const std::string &description);
	const Cmd &FindCommand(const std::string &name);

	void ExecuteCommand(const std::any &userdata, const std::string &cmd);

private:
	Log &log;
	std::unordered_map<std::string, Cmd> cmdmap;
};

#endif
