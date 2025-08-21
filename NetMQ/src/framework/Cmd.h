#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <string>
#include <any>
#include <unordered_map>

#include "CmdArgs.h"

/*
* Class: Cmd
* The command system, responsible for registering any commands that the clients may call.
* Commands are mapped by their name after tokenization, and the server will immediately execute.
* 
*	RegisterCommand: Registers a new command with the system, or modifies an existing one
*	ExecuteCommand: Tokenizes and executes the command string supplied, IOContext as userdata
*/
class Cmd
{
public:
	using CmdFunction = void (*)(const std::any &, const CmdArgs &);

	Cmd();
	~Cmd();

	void RegisterCommand(const std::string &name, const Cmd::CmdFunction &func);
	void ExecuteCommand(const std::any &userdata, const std::string &cmd);

private:
	std::unordered_map<std::string, Cmd::CmdFunction> cmdmap;
};

#endif
