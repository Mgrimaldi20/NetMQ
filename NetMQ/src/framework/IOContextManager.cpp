#include "IOContextManager.h"

IOContextManager::IOContextManager(Log &log)
	: log(log)
{
	log.Info("IO Context Manager started");
}

IOContextManager::~IOContextManager()
{
	log.Info("Shutting down the IO Context Manager");

	Clear();
}

void IOContextManager::Clear()
{
}
