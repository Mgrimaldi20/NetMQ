#ifndef _NETMQ_IOCONTEXTMANAGER_H_
#define _NETMQ_IOCONTEXTMANAGER_H_

#include "Log.h"

class IOContextManager
{
public:
	IOContextManager(Log &log);
	~IOContextManager();

	void Clear();

private:
	Log &log;
};

#endif
