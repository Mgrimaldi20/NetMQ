#ifndef _NETMQ_WINSOCKAPI_H_
#define _NETMQ_WINSOCKAPI_H_

#include <WinSock2.h>
#include <MSWSock.h>

class WinSockAPI
{
public:
	WinSockAPI(unsigned int major, unsigned int minor);
	~WinSockAPI();

private:
	WSAData wsadata;
};

#endif
