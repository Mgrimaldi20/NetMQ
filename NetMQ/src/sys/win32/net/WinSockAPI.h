#ifndef _NETMQ_WINSOCKAPI_H_
#define _NETMQ_WINSOCKAPI_H_

#include <WinSock2.h>
#include <MSWSock.h>

#pragma warning(push)
#pragma warning(disable: 6101)	// header file produces a warning on /W4, and with /WX is an error, hence the need to do all this
#include <WS2tcpip.h>
#pragma warning(pop)

class WinSockAPI
{
public:
	WinSockAPI(unsigned int major, unsigned int minor);
	~WinSockAPI();

private:
	WSAData wsadata;
};

#endif
