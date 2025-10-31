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

template <Log::Type T>
void Log::Write(const std::string &msg)
{
	constexpr auto GetTypeStr = []() consteval
	{
		if constexpr (T == Log::Type::Info) return "INFO";
		else if constexpr (T == Log::Type::Warn) return "WARN";
		else if constexpr (T == Log::Type::Error) return "ERROR";
		else return "UNKNOWN";
	};

	std::scoped_lock lock(logmtx);

	std::format_to(
		std::ostream_iterator<char>(outstream),
		"{} [{}] {}\n",
		std::chrono::floor<std::chrono::seconds>(std::chrono::current_zone()->to_local(std::chrono::system_clock::now())),
		GetTypeStr(),
		msg
	);
}

template void Log::Write<Log::Type::Info>(const std::string &msg);
template void Log::Write<Log::Type::Warn>(const std::string &msg);
template void Log::Write<Log::Type::Error>(const std::string &msg);
