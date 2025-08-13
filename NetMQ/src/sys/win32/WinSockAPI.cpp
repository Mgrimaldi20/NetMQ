#include <iostream>
#include <string>

#include "WinSockAPI.h"

WinSockAPI::WinSockAPI(unsigned int major, unsigned int minor)
	: wsadata()
{
	int ret = WSAStartup(MAKEWORD(major, minor), &wsadata);
	if (ret != 0)
	{
		std::string errmsg("WSAStartup() failed with error: " + ret);
		std::cerr << errmsg << std::endl;
		throw errmsg;
	}
}

WinSockAPI::~WinSockAPI()
{
	WSACleanup();
}
