#include <iostream>
#include <format>
#include <stdexcept>

#include "WinSockAPI.h"

WinSockAPI::WinSockAPI(unsigned int major, unsigned int minor)
	: wsadata()
{
	int ret = WSAStartup(MAKEWORD(major, minor), &wsadata);
	if (ret != 0)
		throw std::runtime_error(std::format("WSAStartup() failed with error: {}", ret));
}

WinSockAPI::~WinSockAPI()
{
	WSACleanup();
}
