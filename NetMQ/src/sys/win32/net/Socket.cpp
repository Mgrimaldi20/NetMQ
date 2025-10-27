#include <stdexcept>
#include <format>

#include "Socket.h"

const std::string GetErrorMessage(const int errcode);

Socket::Socket()
	: socket(INVALID_SOCKET)
{
	socket = WSASocket(AF_INET6, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
		throw std::runtime_error(std::format("WSASocket() failed with error: {}", GetErrorMessage(WSAGetLastError())));

	int zero = 0;
	int ret = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
	if (ret == SOCKET_ERROR)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
		socket = INVALID_SOCKET;
		throw std::runtime_error(std::format("setsockopt(SO_SNDBUF) failed with error: {}", GetErrorMessage(WSAGetLastError())));
	}
}

Socket::~Socket()
{
	if (socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
		socket = INVALID_SOCKET;
	}
}

void Socket::Bind(std::string_view port) const
{
	struct AddrInfo
	{
		AddrInfo()
			: addrlocal(nullptr)
		{
		}

		~AddrInfo()
		{
			if (addrlocal)
				freeaddrinfo(addrlocal);
		}

		addrinfo *addrlocal;
	};

	addrinfo hints = {};
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	AddrInfo addr;

	if (getaddrinfo(nullptr, port.data(), &hints, &addr.addrlocal) != 0)
		throw std::runtime_error(std::format("getaddrinfo() failed with error: {}", GetErrorMessage(WSAGetLastError())));

	if (!addr.addrlocal)
		throw std::runtime_error("getaddrinfo() failed to resolve/convert the interface");

	int ret = bind(socket, addr.addrlocal->ai_addr, (int)addr.addrlocal->ai_addrlen);
	if (ret == SOCKET_ERROR)
		throw std::runtime_error(std::format("bind() failed with error: {}", GetErrorMessage(WSAGetLastError())));
}

void Socket::Listen() const
{
	int ret = listen(socket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
		throw std::runtime_error(std::format("listen() failed with error: {}", GetErrorMessage(WSAGetLastError())));
}

int Socket::Send(WSABUF &wsabuf, OverlappedIO &overlapped)
{
	return WSASend(socket, &wsabuf, 1, nullptr, 0, &overlapped.GetOverlapped(), nullptr);
}

int Socket::Recv(WSABUF &wsabuf, OverlappedIO &overlapped)
{
	DWORD flags = 0;
	return WSARecv(socket, &wsabuf, 1, nullptr, &flags, &overlapped.GetOverlapped(), nullptr);
}

void Socket::CancelIO(OverlappedIO &overlapped) noexcept
{
	CancelIoEx((HANDLE)socket, &overlapped.GetOverlapped());
}

SOCKET &Socket::GetSocket() noexcept
{
	return socket;
}
