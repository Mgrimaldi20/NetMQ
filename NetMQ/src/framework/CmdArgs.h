#ifndef _NETMQ_CMDARGS_H_
#define _NETMQ_CMDARGS_H_

#include <string>
#include <vector>

/*
* Class: CmdArgs
* Turns a command string into a tokenized vector of strings to be consumed by the command system.
* Arguments that are in quotes will be considered as a single string argument
* 
*	operator[]: Returns the argument at the underlying vectors index, [0] is the cmd name
*	GetCount: Returns the number of arguments in the vector
*/
class CmdArgs
{
public:
	const std::string &operator[](size_t index) const;

	CmdArgs(const std::string &cmdstr);
	~CmdArgs();

	const size_t GetCount() const;

private:
	std::vector<std::string> args;
};

#endif
