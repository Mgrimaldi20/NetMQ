#ifndef _NETMQ_LOG_H_
#define _NETMQ_LOG_H_

#include <ostream>
#include <fstream>
#include <filesystem>
#include <format>

class Log
{
public:
	Log();
	Log(const std::filesystem::path &fullpath);
	~Log();

	template <typename ...Args>
	inline void Info(std::format_string<Args...> fmt, Args &&...args)
	{
		Write(Log::Type::Info, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename ...Args>
	inline void Warn(std::format_string<Args...> fmt, Args &&...args)
	{
		Write(Log::Type::Warn, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename ...Args>
	inline void Error(std::format_string<Args...> fmt, Args &&...args)
	{
		Write(Log::Type::Error, std::format(fmt, std::forward<Args>(args)...));
	}

private:
	enum class Type
	{
		Info,
		Warn,
		Error
	};

	void Write(Log::Type type, const std::string &msg);

	std::ofstream logfile;
	std::ostream &outstream;
};

#endif
