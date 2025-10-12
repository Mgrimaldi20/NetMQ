#ifndef _NETMQ_SOCKET_H_
#define _NETMQ_SOCKET_H_

#include "../io/OverlappedIO.h"

#include "WinSockAPI.h"

class Socket
{
public:
	static constexpr std::string_view NET_DEFAULT_PORT = "5001";

	Socket();
	~Socket();

	void Bind(const std::string_view port = NET_DEFAULT_PORT) const;
	void Listen() const;

	void CancelIO(OverlappedIO &overlapped) noexcept;

	SOCKET &GetSocket() noexcept;

private:
	SOCKET socket;
};

#endif
