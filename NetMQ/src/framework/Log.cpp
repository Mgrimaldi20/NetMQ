#include <iostream>
#include <chrono>

#include "Log.h"

Log::Log()
	: outstream(std::cout)
{
}

Log::Log(const std::filesystem::path &fullpath)
	: outstream(logfile)
{
	if (fullpath.empty())
		return;

	if (!fullpath.has_filename())
		return;

	if (fullpath.has_parent_path())
	{
		if (!std::filesystem::create_directories(fullpath.parent_path()))	// create if the dirs dont exist
			return;
	}

	logfile.open(fullpath.string());
}

Log::~Log()
{
	std::cout << "Closing log file" << std::endl;
}

void Log::Write(Log::Type type, const std::string &msg)
{
	std::format_to(std::ostream_iterator<char>(outstream), "{} [{}] {}\n",
		std::chrono::floor<std::chrono::seconds>(std::chrono::current_zone()->to_local(std::chrono::system_clock::now())),
		[](Log::Type type)
		{
			switch (type)
			{
				case Log::Type::Info: return "INFO";
				case Log::Type::Warn: return "WARN";
				case Log::Type::Error: return "ERROR";
				default: return "UNKNOWN";
			}
		} (type),
		msg
	);
}
