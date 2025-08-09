#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>

#include <iostream>
#include <thread>

static constexpr unsigned short NET_DEFAULT_PORT = 5001;
static constexpr unsigned int NET_DEFAULT_THREADS = 2;
static constexpr size_t NET_MAX_BUFFER_SIZE = 8192;

enum class IOOperation
{
	IOOP_ACCEPT,
	IOOP_READ,
	IOOP_WRITE
};

struct IOContext
{
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	SOCKET acceptsocket;
	IOOperation ioop;
	char buffer[NET_MAX_BUFFER_SIZE];
};

SOCKET CreateSocket();
bool CreateListenSocket(const SOCKET &listensocket);
bool CreateAcceptSocket(const SOCKET &listensocket, const HANDLE &iocp, bool updateiocp, LPFN_ACCEPTEX &AcceptExFn);

int main(int argc, char **argv)
{
	(int)argc;
	(char **)argv;

	unsigned int numthreads = std::thread::hardware_concurrency();
	numthreads = (numthreads == 0) ? NET_DEFAULT_THREADS : numthreads;

	std::cout << "Number of threads available: " << numthreads << std::endl;

	WSAData wsadata = {};
	int ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (ret != 0)
	{
		std::cerr << "WSAStartup() failed with error: " << ret;
		return 1;
	}

	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (!iocp)
	{
		std::cerr << "CreateIoCompletionPort() failed with error (initial): " << GetLastError();
		WSACleanup();
		return 1;
	}

	SOCKET listensocket = CreateSocket();
	if (listensocket == INVALID_SOCKET)
	{
		std::cerr << "CreateSocket() failed";
		CloseHandle(iocp);
		WSACleanup();
		return 1;
	}

	if (!CreateListenSocket(listensocket))
	{
		std::cerr << "CreateListenSocket() failed";
		closesocket(listensocket);
		CloseHandle(iocp);
		WSACleanup();
		return 1;
	}

	LPFN_ACCEPTEX AcceptExFn = nullptr;
	if (!CreateAcceptSocket(listensocket, iocp, true, AcceptExFn))
	{
		std::cerr << "CreateListenSocket() failed";
		closesocket(listensocket);
		CloseHandle(iocp);
		WSACleanup();
		return 1;
	}

	auto WorkerThread = [&iocp, &listensocket, &AcceptExFn]() -> void
	{
		DWORD iosize = 0;
		WSAOVERLAPPED *wsaoverlapped = nullptr;
		ULONG_PTR completionkey;

		while (GetQueuedCompletionStatus(iocp, &iosize, &completionkey, &wsaoverlapped, INFINITE))
		{
			IOContext *ioctx = CONTAINING_RECORD(wsaoverlapped, IOContext, overlapped);
			switch (ioctx->ioop)
			{
				case IOOperation::IOOP_ACCEPT:
				{
					// after AcceptEx completed
					int ret = setsockopt(ioctx->acceptsocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&listensocket, sizeof(listensocket));
					if (ret == SOCKET_ERROR)
					{
						std::cerr << "setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket: " << WSAGetLastError();
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						return;
					}

					if (!CreateIoCompletionPort((HANDLE)ioctx->acceptsocket, iocp, (ULONG_PTR)ioctx->acceptsocket, 0))
					{
						std::cerr << "CreateIoCompletionPort() failed to associate iocp handle to accept socket: " << GetLastError();
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						return;
					}

					// start a read from a new client
					ioctx->ioop = IOOperation::IOOP_READ;
					ioctx->wsabuf.len = NET_MAX_BUFFER_SIZE;

					DWORD flags = 0;
					ret = WSARecv(ioctx->acceptsocket, &ioctx->wsabuf, 1, nullptr, &flags, &ioctx->overlapped, nullptr);
					if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
					{
						std::cerr << "WSARecv() failed with error: " << WSAGetLastError();
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						return;
					}

					// post another accept for a new client
					if (!CreateAcceptSocket(listensocket, iocp, false, AcceptExFn))
					{
						std::cerr << "CreateAcceptSocket() failed";
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						return;
					}

					break;
				}

				case IOOperation::IOOP_READ:
				{
					if (iosize == 0)	// client closed
					{
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						continue;
					}

					// echo back to the client
					ioctx->ioop = IOOperation::IOOP_WRITE;
					ioctx->wsabuf.len = iosize;

					int ret = WSASend(ioctx->acceptsocket, &ioctx->wsabuf, 1, nullptr, 0, &ioctx->overlapped, nullptr);
					if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
					{
						std::cerr << "WSASend() failed with error: " << WSAGetLastError();
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						return;
					}

					break;
				}

				case IOOperation::IOOP_WRITE:
				{
					// post another read after sending
					ioctx->ioop = IOOperation::IOOP_READ;
					ioctx->wsabuf.len = NET_MAX_BUFFER_SIZE;

					DWORD flags = 0;
					int ret = WSARecv(ioctx->acceptsocket, &ioctx->wsabuf, 1, nullptr, &flags, &ioctx->overlapped, nullptr);
					if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
					{
						std::cerr << "WSARecv() failed with error: " << WSAGetLastError();
						closesocket(ioctx->acceptsocket);
						delete ioctx;
						return;
					}

					break;
				}
			}
		}
	};

	for (unsigned int i=0; i<numthreads; i++)
		std::thread(WorkerThread).detach();

	std::cout << "Echo server running on port " << NET_DEFAULT_PORT << std::endl;
	std::cout << "Press ENTER to exit..." << std::endl;
	std::cin.get();

	closesocket(listensocket);
	CloseHandle(iocp);
	WSACleanup();

	return 0;
}

SOCKET CreateSocket()
{
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		std::cerr << "WSASocket() failed with error: " << WSAGetLastError();
		return socket;
	}

	int zero = 0;
	int ret = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "setsockopt(SO_SNDBUF) failed with error: " << WSAGetLastError();
		return socket;
	}

	return socket;
}

bool CreateListenSocket(const SOCKET &listensocket)
{
	sockaddr_in hints = {};
	hints.sin_family = AF_INET;
	hints.sin_port = htons(NET_DEFAULT_PORT);
	hints.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(listensocket, (sockaddr *)&hints, sizeof(hints));
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "bind() failed with error: " << WSAGetLastError();
		return false;
	}

	ret = listen(listensocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "listen() failed with error: " << WSAGetLastError();
		return false;
	}

	return true;
}

bool CreateAcceptSocket(const SOCKET &listensocket, const HANDLE &iocp, bool updateiocp, LPFN_ACCEPTEX &AcceptExFn)
{
	if (updateiocp)
	{
		if (!CreateIoCompletionPort((HANDLE)listensocket, iocp, 0, 0))
		{
			std::cerr << "CreateIoCompletionPort() failed to associate iocp handle to listen socket: " << GetLastError();
			return false;
		}

		GUID acceptexguid = WSAID_ACCEPTEX;
		DWORD bytes = 0;

		int ret = WSAIoctl(
			listensocket,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&acceptexguid,
			sizeof(acceptexguid),
			&AcceptExFn,
			sizeof(AcceptExFn),
			&bytes,
			nullptr,
			nullptr
		);

		if (ret == SOCKET_ERROR)
		{
			std::cerr << "WSAIoctl() failed to retrieve AcceptEx function address with error: " << WSAGetLastError();
			return false;
		}
	}

	IOContext *ioctx = new IOContext();
	ZeroMemory(&ioctx->overlapped, sizeof(ioctx->overlapped));
	ioctx->ioop = IOOperation::IOOP_ACCEPT;
	ioctx->wsabuf.buf = ioctx->buffer;
	ioctx->wsabuf.len = NET_MAX_BUFFER_SIZE;

	ioctx->acceptsocket = CreateSocket();
	if (ioctx->acceptsocket == INVALID_SOCKET)
	{
		std::cerr << "CreateListenSocket() failed";
		delete ioctx;
		return false;
	}

	DWORD recvbytes = 0;
	int ret = AcceptExFn(listensocket,
		ioctx->acceptsocket,
		ioctx->buffer,
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&recvbytes,
		&ioctx->overlapped
	);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "AcceptEx() failed with error: " << WSAGetLastError();
		delete ioctx;
		return false;
	}

	return true;
}
