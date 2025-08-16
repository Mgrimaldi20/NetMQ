#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <string>
#include <unordered_map>
#include <queue>

#include "CmdArgs.h"

/*
* Class: Cmd
* The command system, responsible for registering any commands that the clients may call.
* For now, the commands are queued from the worker threads and continuously executed.
* The commands are stored in a queue, but may change this to store them in a string buffer.
* May evntually do away with buffering and instead execute commands as they come for performance
* 
*	RegisterCommand: Registers a new command with the system, or modifies an existing one
*	BufferCommand: Pushes a command onto the queue with its parameters to be eventually executed
*/
class Cmd
{
public:
	typedef void (*CmdFunction_t)(const CmdArgs &);
	typedef std::unordered_map<std::string, Cmd::CmdFunction_t> maptype_t;

	Cmd(Cmd::maptype_t &cmdmap);
	~Cmd();

	void RegisterCommand(const std::string &name, const Cmd::CmdFunction_t &func);
	void BufferCommand(const std::string &cmd);
	void ExecuteCommandBuffer();

private:
	Cmd::maptype_t &cmdmap;
	std::queue<std::string> cmdqueue;
};

#endif
