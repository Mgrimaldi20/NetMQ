#ifndef _NETMQ_LOG_H_
#define _NETMQ_LOG_H_

#include <ostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <format>
#include <mutex>

/*
* Class: Log
* The logging system, controls wiritng to a log file or to std::cout streams.
* Different logging levels are provided to represent the class of information to log.
* The logger will print the current time, level, and message.
* 
*	Info: log a formatted information message, designed for general program information and state
*	Warn: log a formatted warning message, designed for recoverable issues or abnormal state
*	Error: log a formatted error message, designed for unrecoverable code errors, program should quit
*/
class Log
{
public:
	Log();
	Log(const std::filesystem::path &fullpath);
	~Log();

	template<typename ...Args>
	inline void Info(std::format_string<Args...> fmt, Args &&...args);

	template<typename ...Args>
	inline void Warn(std::format_string<Args...> fmt, Args &&...args);

	template<typename ...Args>
	inline void Error(std::format_string<Args...> fmt, Args &&...args);

private:
	enum class Type
	{
		Info,
		Warn,
		Error
	};

	template<Log::Type T>
	void Write(const std::string &msg);

	std::ofstream logfile;
	std::ostream &outstream;
	std::string logname;

	std::mutex logmtx;
};

template<typename ...Args>
inline void Log::Info(std::format_string<Args...> fmt, Args && ...args)
{
	Write<Log::Type::Info>(std::format(fmt, std::forward<Args>(args)...));
}

template<typename ...Args>
inline void Log::Warn(std::format_string<Args...> fmt, Args && ...args)
{
	Write<Log::Type::Warn>(std::format(fmt, std::forward<Args>(args)...));
}

template<typename ...Args>
inline void Log::Error(std::format_string<Args...> fmt, Args && ...args)
{
	Write<Log::Type::Error>(std::format(fmt, std::forward<Args>(args)...));
}

#endif
