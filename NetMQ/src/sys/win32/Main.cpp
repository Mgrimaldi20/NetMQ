#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "WinSockAPI.h"

#include <iostream>
#include <system_error>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <list>

#include "framework/Cmd.h"

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
	IOContext()
		: overlapped(),
		wsabuf(),
		acceptsocket(INVALID_SOCKET),
		ioop(),
		buffer(),
		iter()
	{
	}

	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	SOCKET acceptsocket;
	IOOperation ioop;
	char buffer[NET_MAX_BUFFER_SIZE];
	std::list<IOContext>::iterator iter;
	std::unordered_set<std::string> subscriptions;
};

SOCKET CreateSocket();
bool CreateListenSocket(const SOCKET &listensocket);
bool CreateAcceptSocket(const SOCKET &listensocket, const HANDLE &iocp, const bool updateiocp);
void CloseClient(IOContext *context);

void Publish_Cmd(const std::any userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if ((argc < 3) || (argc > 3))
	{
		std::cout << "Usage: " << args[0] << " [topic] [value]" << std::endl;
		return;
	}

	std::cout << "Test function PUB: "
		<< std::any_cast<IOContext *>(userdata)->acceptsocket << " "
		<< args[0] << " "
		<< args[1] << " "
		<< args[2] << std::endl;
}

void Subscribe_Cmd(const std::any userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if ((argc < 2) || (argc > 2))
	{
		std::cout << "Usage: " << args[0] << " [topic]" << std::endl;
		return;
	}

	IOContext *ioctx = std::any_cast<IOContext *>(userdata);

	std::cout << "Test function SUB: "
		<< ioctx->acceptsocket << " "
		<< args[0] << " "
		<< args[1] << std::endl;

	ioctx->subscriptions.insert(args[1]);
}

void Unsubscribe_Cmd(const std::any userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if ((argc < 2) || (argc > 2))
	{
		std::cout << "Usage: " << args[0] << " [topic]" << std::endl;
		return;
	}

	IOContext *ioctx = std::any_cast<IOContext *>(userdata);

	std::cout << "Test function UNSUB: "
		<< ioctx->acceptsocket << " "
		<< args[0] << " "
		<< args[1] << std::endl;

	ioctx->subscriptions.erase(args[1]);
}

LPFN_ACCEPTEX AcceptExFn;
std::list<IOContext> ioctxlist;
std::recursive_mutex ioctxlistmtx;

int main(int argc, char **argv)
{
	(int)argc;
	(char **)argv;

	Cmd::maptype_t cmdmap;
	Cmd cmd(cmdmap);

	cmd.RegisterCommand("pub", Publish_Cmd);
	cmd.RegisterCommand("sub", Subscribe_Cmd);
	cmd.RegisterCommand("unsub", Unsubscribe_Cmd);

	WinSockAPI wsa(2, 2);

	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (!iocp)
	{
		std::cerr << "CreateIoCompletionPort() failed with error (initial): " << std::system_category().message(GetLastError()) << std::endl;
		return 1;
	}

	SOCKET listensocket = CreateSocket();
	if (listensocket == INVALID_SOCKET)
	{
		std::cerr << "CreateSocket() failed" << std::endl;
		CloseHandle(iocp);
		return 1;
	}

	if (!CreateListenSocket(listensocket))
	{
		std::cerr << "CreateListenSocket() failed" << std::endl;
		closesocket(listensocket);
		CloseHandle(iocp);
		return 1;
	}

	if (!CreateAcceptSocket(listensocket, iocp, true))
	{
		std::cerr << "CreateListenSocket() failed" << std::endl;
		closesocket(listensocket);
		CloseHandle(iocp);
		return 1;
	}

	bool stopthreads = false;

	auto WorkerThread = [&iocp, &listensocket, &stopthreads, &cmd]() -> void
	{
		DWORD iosize = 0;
		ULONG_PTR completionkey = 0;
		WSAOVERLAPPED *wsaoverlapped = nullptr;

		while (true)
		{
			bool status = GetQueuedCompletionStatus(iocp, &iosize, &completionkey, &wsaoverlapped, INFINITE);
			if (!status)
				std::cerr << "GetQueuedCompletionStatus() failed with error: " << std::system_category().message(GetLastError()) << std::endl;

			if (!completionkey && !wsaoverlapped)
				break;

			if (!status && !wsaoverlapped)
				break;

			IOContext *ioctx = CONTAINING_RECORD(wsaoverlapped, IOContext, overlapped);	// determine the clients socket context, and what action they want to take
			switch (ioctx->ioop)
			{
				case IOOperation::IOOP_ACCEPT:
				{
					// after AcceptEx completed
					int ret = setsockopt(ioctx->acceptsocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&listensocket, sizeof(listensocket));
					if (ret == SOCKET_ERROR)
					{
						std::cerr << "setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket: " << std::system_category().message(WSAGetLastError()) << std::endl;
						CloseClient(ioctx);
						return;
					}

					if (!CreateIoCompletionPort((HANDLE)ioctx->acceptsocket, iocp, (ULONG_PTR)ioctx->acceptsocket, 0))
					{
						std::cerr << "CreateIoCompletionPort() failed to associate iocp handle to accept socket: " << std::system_category().message(GetLastError()) << std::endl;
						CloseClient(ioctx);
						return;
					}

					// start a read from a new client
					ioctx->ioop = IOOperation::IOOP_READ;
					ioctx->wsabuf.len = NET_MAX_BUFFER_SIZE;

					DWORD flags = 0;
					ret = WSARecv(ioctx->acceptsocket, &ioctx->wsabuf, 1, nullptr, &flags, &ioctx->overlapped, nullptr);
					if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
					{
						std::cerr << "WSARecv() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
						CloseClient(ioctx);
					}

					// post another accept for a new client if the server isnt stopping
					if (!stopthreads)
					{
						if (!CreateAcceptSocket(listensocket, iocp, false))
						{
							std::cerr << "CreateAcceptSocket() failed" << std::endl;
							CloseClient(ioctx);
							return;
						}
					}

					break;
				}

				case IOOperation::IOOP_READ:
				{
					if (iosize == 0)	// client closed
					{
						CloseClient(ioctx);
						continue;
					}

					// echo back to the client
					ioctx->ioop = IOOperation::IOOP_WRITE;
					ioctx->wsabuf.len = iosize;

					int ret = WSASend(ioctx->acceptsocket, &ioctx->wsabuf, 1, nullptr, 0, &ioctx->overlapped, nullptr);
					if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
					{
						std::cerr << "WSASend() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
						CloseClient(ioctx);
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
						std::cerr << "WSARecv() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
						CloseClient(ioctx);
					}

					cmd.ExecuteCommand(ioctx, ioctx->wsabuf.buf);
					memset(ioctx->buffer, '\0', NET_MAX_BUFFER_SIZE);	// clear the buffer of garbage data

					break;
				}

				default:
					break;
			}
		}
	};

	unsigned int numthreads = std::thread::hardware_concurrency() * 2;
	numthreads = (numthreads == 0) ? NET_DEFAULT_THREADS : numthreads;

	if (numthreads == NET_DEFAULT_THREADS)
	{
		std::cout << "The number of threads avaliable is equal to the default number ["
			<< NET_DEFAULT_THREADS
			<< "]: If this is not correct, you may wish to restart NetMQ as the correct number of system threads have not been detected"
			<< std::endl;
	}

	std::vector<std::thread> threads;
	for (unsigned int i=0; i<numthreads; i++)
		threads.emplace_back(WorkerThread);

	std::cout << "Number of threads available: " << numthreads << std::endl;
	std::cout << "Echo server running on port " << NET_DEFAULT_PORT << std::endl;
	std::cout << "Press ENTER to exit..." << std::endl;
	std::cin.get();

	stopthreads = true;

	closesocket(listensocket);		// stop accepting new connections and cause any accepts to fail

	{
		std::scoped_lock<std::recursive_mutex> lock(ioctxlistmtx);
		for (IOContext &ioctx : ioctxlist)
		{
			if (ioctx.acceptsocket != INVALID_SOCKET)
			{
				shutdown(ioctx.acceptsocket, SD_BOTH);
				CancelIoEx((HANDLE)ioctx.acceptsocket, &ioctx.overlapped);
			}
		}

		if (!ioctxlist.empty())
		{
			for (IOContext &ioctx : ioctxlist)
			{
				std::cout << "Closing client: " << ioctx.acceptsocket << std::endl;

				if (ioctx.acceptsocket == INVALID_SOCKET)
				{
					std::cerr << "Socket context is alread INVALID" << std::endl;
					continue;
				}

				closesocket(ioctx.acceptsocket);
			}
		}
	}

	for (unsigned int i=0; i<numthreads; i++)
		PostQueuedCompletionStatus(iocp, 0, 0, nullptr);

	for (std::thread &thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}

	CloseHandle(iocp);

	return 0;
}

SOCKET CreateSocket()
{
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		std::cerr << "WSASocket() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
		return socket;
	}

	int zero = 0;
	int ret = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "setsockopt(SO_SNDBUF) failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
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
		std::cerr << "bind() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
		return false;
	}

	ret = listen(listensocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "listen() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
		return false;
	}

	return true;
}

bool CreateAcceptSocket(const SOCKET &listensocket, const HANDLE &iocp, const bool updateiocp)
{
	std::scoped_lock<std::recursive_mutex> lock(ioctxlistmtx);

	if (updateiocp)
	{
		if (!CreateIoCompletionPort((HANDLE)listensocket, iocp, 0, 0))
		{
			std::cerr << "CreateIoCompletionPort() failed to associate iocp handle to listen socket: " << std::system_category().message(GetLastError()) << std::endl;
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
			std::cerr << "WSAIoctl() failed to retrieve AcceptEx function address with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
			return false;
		}
	}

	std::list<IOContext>::iterator iter = ioctxlist.emplace(ioctxlist.end());

	IOContext &ioctx = *iter;
	ioctx.iter = iter;

	ZeroMemory(&ioctx.overlapped, sizeof(ioctx.overlapped));
	ioctx.ioop = IOOperation::IOOP_ACCEPT;
	ioctx.wsabuf.buf = ioctx.buffer;
	ioctx.wsabuf.len = NET_MAX_BUFFER_SIZE;

	ioctx.acceptsocket = CreateSocket();
	if (ioctx.acceptsocket == INVALID_SOCKET)
	{
		std::cerr << "CreateListenSocket() failed" << std::endl;
		ioctxlist.erase(iter);
		return false;
	}

	DWORD recvbytes = 0;
	int ret = AcceptExFn(listensocket,
		ioctx.acceptsocket,
		ioctx.buffer,
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&recvbytes,
		&ioctx.overlapped
	);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "AcceptEx() failed with error: " << std::system_category().message(WSAGetLastError()) << std::endl;
		closesocket(ioctx.acceptsocket);
		ioctxlist.erase(iter);
		return false;
	}

	return true;
}

void CloseClient(IOContext *context)
{
	std::scoped_lock<std::recursive_mutex> lock(ioctxlistmtx);

	if (!context)
		return;

	std::cout << "Closing client: " << context->acceptsocket << std::endl;

	if (context->acceptsocket == INVALID_SOCKET)
	{
		std::cerr << "Socket context is alread INVALID" << std::endl;
		return;
	}

	closesocket(context->acceptsocket);

	std::list<IOContext>::iterator iter = context->iter;
	ioctxlist.erase(iter);
}
