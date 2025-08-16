#include <sstream>
#include <iomanip>

#include "CmdArgs.h"

const std::string &CmdArgs::operator[](size_t index) const
{
	return this->args.at(index);
}

CmdArgs::CmdArgs(const std::string &cmdstr)
{
	std::istringstream iss(cmdstr);
	std::string token;

	while (iss >> std::quoted(token))
		args.push_back(token);
}

CmdArgs::~CmdArgs()
{
}

const size_t CmdArgs::GetCount() const
{
	return args.size();
}
