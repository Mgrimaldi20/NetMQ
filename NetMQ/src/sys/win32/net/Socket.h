#ifndef _NETMQ_SOCKET_H_
#define _NETMQ_SOCKET_H_

#include "../io/OverlappedIO.h"

#include "WinSockAPI.h"

class Socket
{
public:
	Socket();
	~Socket();

	void Bind(const std::string_view port) const;
	void Listen() const;

	void CancelIO(OverlappedIO &overlapped) noexcept;

	SOCKET &GetSocket() noexcept;

private:
	SOCKET socket;
};

#endif
