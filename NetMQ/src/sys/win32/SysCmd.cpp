#include "../SysCmd.h"

void ConnectSysCmd::operator()(std::shared_ptr<IOContext> ioctx) const
{
	(void)ioctx;
}

void PublishSysCmd::operator()(std::shared_ptr<IOContext> ioctx) const
{
	(void)ioctx;
}

void SubscribeSysCmd::operator()(std::shared_ptr<IOContext> ioctx) const
{
	(void)ioctx;
}

void UnsubscribeSysCmd::operator()(std::shared_ptr<IOContext> ioctx) const
{
	(void)ioctx;
}

void DisconnectSysCmd::operator()(std::shared_ptr<IOContext> ioctx) const
{
	(void)ioctx;
}
