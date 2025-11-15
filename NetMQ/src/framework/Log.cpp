#include <iostream>
#include <chrono>
#include <stdexcept>

#include "Log.h"

Log::Log()
	: outstream(std::cout),
	logname("COUT"),
	logmtx()
{
	Info("Log opened: {}", logname);
}

Log::Log(const std::filesystem::path &fullpath)
	: outstream(logfile),
	logmtx()
{
	if (fullpath.empty())
		throw std::runtime_error("The full path provided to the Logger is empty");

	if (!fullpath.has_filename())
		throw std::runtime_error("The full path provided to the Logger has no file name");

	if (fullpath.has_parent_path())
		std::filesystem::create_directories(fullpath.parent_path());

	logfile.open(fullpath.string());

	logname = fullpath.filename().string();

	Info("Log opened: {}", logname);
}

Log::~Log()
{
	Info("Closing log: {}", logname);

	outstream.flush();
}
