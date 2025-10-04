#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "net/WinSockAPI.h"

#include <iostream>
#include <cstddef>
#include <system_error>
#include <stdexcept>
#include <string>
#include <string_view>
#include <format>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <unordered_set>
#include <list>
#include <vector>
#include <span>
#include <any>
#include <memory>

#include "framework/Log.h"
#include "framework/CmdSystem.h"

constexpr std::string_view NET_DEFAULT_PORT = "5001";
constexpr unsigned int NET_DEFAULT_THREADS = 2;
constexpr size_t NET_MAX_BUFFER_SIZE = 8192;

const std::string GetErrorMessage(const int errcode);
bool ValidateOptions(int argc, char **argv);
BOOL WINAPI CtrlHandler(DWORD event);

struct Socket
{
	Socket()
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

	~Socket()
	{
		if (socket != INVALID_SOCKET)
		{
			shutdown(socket, SD_BOTH);
			closesocket(socket);
			socket = INVALID_SOCKET;
		}
	}

	void Bind(const std::string_view port) const
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

	void Listen() const
	{
		int ret = listen(socket, SOMAXCONN);
		if (ret == SOCKET_ERROR)
			throw std::runtime_error(std::format("listen() failed with error: {}", GetErrorMessage(WSAGetLastError())));
	}

	const SOCKET &GetSocket() const
	{
		return socket;
	}

	SOCKET socket;
};

struct IOCompletionPort
{
	IOCompletionPort()
		: iocp(INVALID_HANDLE_VALUE)
	{
		iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (!iocp)
			throw std::runtime_error(std::format("CreateIoCompletionPort() failed with error (initial): {}", GetErrorMessage(GetLastError())));
	}

	~IOCompletionPort()
	{
		if (iocp && iocp != INVALID_HANDLE_VALUE)
		{
			CloseHandle(iocp);
			iocp = INVALID_HANDLE_VALUE;
		}
	}

	bool UpdateIOCompletionPort(const Socket &socket, ULONG_PTR completionkey) const
	{
		if (!CreateIoCompletionPort((HANDLE)socket.GetSocket(), iocp, completionkey, 0))
			return false;

		return true;
	}

	bool GetQueuedCompletionStatus(unsigned long *iosize, unsigned long long *completionkey, WSAOVERLAPPED **wsaoverlapped)
	{
		return ::GetQueuedCompletionStatus(iocp, iosize, completionkey, wsaoverlapped, INFINITE);
	}

	bool PostQueuedQuitStatus()
	{
		return PostQueuedCompletionStatus(iocp, 0, 0, nullptr);
	}

	HANDLE iocp;
};

struct IOContext;

enum class IOOperation
{
	Accept,
	Read,
	Write
};

struct OverlappedIO
{
	OverlappedIO(IOOperation ioop, std::weak_ptr<IOContext> ioctx)
		: ioop(ioop),
		ioctx(ioctx)
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	std::shared_ptr<IOContext> GetIOContext() const
	{
		return ioctx.lock();
	}

	WSAOVERLAPPED overlapped;
	IOOperation ioop;
	std::weak_ptr<IOContext> ioctx;
};

struct IOContext
{
	IOContext()
		: acceptsocket(),
		acceptov(IOOperation::Accept, {}),
		recvov(IOOperation::Read, {}),
		sendov(IOOperation::Write, {}),
		recvwsabuf(),
		sendwsabuf(),
		recving(false),
		sending(false),
		buffer(NET_MAX_BUFFER_SIZE),
		subscriptions(),
		outgoing(),
		acceptbuffer((sizeof(SOCKADDR_STORAGE) + 16) * 2),
		closing(false)
	{
		outgoing.reserve(100);	// this should be configurable based on the workload
	}

	~IOContext()
	{
		CancelOverlappedIO();
	}

	void Init(std::weak_ptr<IOContext> self)
	{
		acceptov.ioctx = self;
		recvov.ioctx = self;
		sendov.ioctx = self;
	}

	void CancelOverlappedIO()
	{
		CancelIoEx((HANDLE)acceptsocket.GetSocket(), &acceptov.overlapped);
		CancelIoEx((HANDLE)acceptsocket.GetSocket(), &sendov.overlapped);
		CancelIoEx((HANDLE)acceptsocket.GetSocket(), &recvov.overlapped);
	}

	Socket acceptsocket;

	OverlappedIO acceptov;
	OverlappedIO recvov;
	OverlappedIO sendov;

	WSABUF recvwsabuf;
	WSABUF sendwsabuf;

	std::atomic<bool> recving;
	std::atomic<bool> sending;

	std::vector<std::byte> buffer;
	std::vector<std::byte> outgoing;
	std::vector<std::byte> acceptbuffer;

	std::unordered_set<std::string> subscriptions;

	std::atomic<bool> closing;
};

void WorkerThread(IOCompletionPort &iocp, Socket &listensocket, CmdSystem &cmd, Log &log);

bool GetAcceptExFnPtr(const Socket &listensocket);

bool PostAccept(const Socket &listensocket);
void PostRecv(std::shared_ptr<IOContext>);
void PostSend(std::shared_ptr<IOContext>);

void CloseClient(std::shared_ptr<IOContext>);

LPFN_ACCEPTEX AcceptExFn;

std::list<std::shared_ptr<IOContext>> ioctxlist;
std::mutex ioctxlistmtx;

std::atomic<bool> endserver;
std::atomic<bool> restartserver;
std::condition_variable cleanupcv;

int main(int argc, char **argv)
{
	if (!ValidateOptions(argc, argv))
		return 1;

	Log log;
	CmdSystem cmd(log);

	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
	{
		log.Error("SetConsoleCtrlHandler() failed to install console handler: {}", GetErrorMessage(GetLastError()));
		return 1;
	}

	WinSockAPI wsa(2, 2);

	restartserver = true;

	while (restartserver.load())
	{
		endserver = false;
		restartserver = false;

		IOCompletionPort iocp;

		Socket listensocket;

		listensocket.Bind(NET_DEFAULT_PORT);
		listensocket.Listen();

		if (!iocp.UpdateIOCompletionPort(listensocket, 0))
		{
			log.Error("UpdateIOCompletionPort() failed to associate iocp handle to listen socket: {}", GetErrorMessage(GetLastError()));
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		if (!GetAcceptExFnPtr(listensocket))
		{
			log.Error("GetAcceptExFnPtr() failed");
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		if (!PostAccept(listensocket))
		{
			log.Error("PostAccept() failed (initial)");
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		unsigned int numthreads = std::thread::hardware_concurrency() * 2;
		numthreads = (numthreads == 0) ? NET_DEFAULT_THREADS : numthreads;

		if (numthreads == NET_DEFAULT_THREADS)
		{
			log.Info("The number of threads avaliable is equal to the default number [{}]", NET_DEFAULT_THREADS);
			log.Info("If this is not correct, you may wish to restart NetMQ as the correct number of system threads have not been detected");
		}

		std::vector<std::thread> threads;
		for (unsigned int i=0; i<numthreads; i++)
			threads.emplace_back(WorkerThread, std::ref(iocp), std::ref(listensocket), std::ref(cmd), std::ref(log));

		log.Info("Number of threads available: {}", numthreads);
		log.Info("NetMQ server running on port: {}", NET_DEFAULT_PORT);
		log.Info("Press Ctrl-C to exit, or Ctrl-Break to restart...");

		{
			std::mutex cvmtx;
			std::unique_lock lock(cvmtx);
			cleanupcv.wait(lock, [] { return endserver.load(); });
		}

		endserver = true;

		for (unsigned int i=0; i<numthreads; i++)
			iocp.PostQueuedQuitStatus();

		for (std::thread &thread : threads)
		{
			if (thread.joinable())
				thread.join();
		}

		size_t count = ioctxlist.size();
		ioctxlist.clear();
		log.Info("Cleared {} contexts from list", count);
		
		if (restartserver.load())
			log.Info("NetMQ is restarting...\n");

		else
			log.Info("NetMQ is exiting...");
	}

	SetConsoleCtrlHandler(CtrlHandler, FALSE);

	return 0;
}

const std::string GetErrorMessage(const int errcode)
{
	std::error_condition errcond = std::system_category().default_error_condition(errcode);

	return std::format(
		"[Category: {}, Value: {}, Message: {}]",
		errcond.category().name(),
		errcond.value(),
		errcond.message()
	);
}

bool ValidateOptions(int argc, char **argv)
{
	for (int i=1; i<argc; i++)
	{
		if (argv[i][0] != '-')
			continue;

		switch (argv[i][1])
		{
			case 'p':
				break;

			case '?':
				std::cout << std::endl << "Usage:" << std::endl
					<< "NetMQ [-p:<port>] [-?]" << std::endl
					<< "--------------------------------------------------" << std::endl
					<< "-p:<port>    Specify the port number of the server" << std::endl
					<< "-?           Prints out this help message and exit" << std::endl;

				return false;

			default:
				std::cout << "Unknown command line options flag: " << argv[i] << std::endl;
				return false;
		}
	}

	return true;
}

BOOL WINAPI CtrlHandler(DWORD event)
{
	switch (event)
	{
		case CTRL_BREAK_EVENT:
			restartserver = true;
			[[fallthrough]];

		case CTRL_C_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
		case CTRL_CLOSE_EVENT:
			endserver = true;
			cleanupcv.notify_all();
			break;

		default:
			return FALSE;	// pass onto the next or default ctrl handler
	}

	return TRUE;
}

void WorkerThread(IOCompletionPort &iocp, Socket &listensocket, CmdSystem &cmd, Log &log)
{
	DWORD iosize = 0;
	ULONG_PTR completionkey = 0;
	WSAOVERLAPPED *wsaoverlapped = nullptr;

	while (true)
	{
		bool status = iocp.GetQueuedCompletionStatus(&iosize, &completionkey, &wsaoverlapped);
		if (!status)
			log.Error("IOCompletionPort::GetQueuedCompletionStatus() failed with error: {}", GetErrorMessage(GetLastError()));

		if ((!completionkey && !wsaoverlapped) || (!status && !wsaoverlapped))
			break;

		if (!wsaoverlapped)
			continue;

		OverlappedIO *perio = reinterpret_cast<OverlappedIO *>(wsaoverlapped);
		std::shared_ptr<IOContext> ioctx = perio->GetIOContext();

		if (!ioctx)
		{
			log.Warn("Unknown IO Context, it may have already been closed...");
			continue;
		}

		switch (perio->ioop)
		{
			case IOOperation::Accept:
			{
				// after AcceptEx completed
				SOCKET ls = listensocket.GetSocket();
				int ret = setsockopt(ioctx->acceptsocket.GetSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char *>(&ls), sizeof(ls));
				if (ret == SOCKET_ERROR)
				{
					log.Error("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket: {}", GetErrorMessage(WSAGetLastError()));
					CloseClient(ioctx);
					return;
				}

				if (!iocp.UpdateIOCompletionPort(ioctx->acceptsocket, static_cast<ULONG_PTR>(ioctx->acceptsocket.GetSocket())))
				{
					log.Error("UpdateIOCompletionPort() failed to associate iocp handle to accept socket: {}", GetErrorMessage(GetLastError()));
					CloseClient(ioctx);
					return;
				}

				// start a read from a new client
				PostRecv(ioctx);

				// post another accept for a new client if the server isnt stopping
				if (!endserver.load())
				{
					if (!PostAccept(listensocket))
					{
						log.Error("PostAccept() failed to post an accept for a new client");
						CloseClient(ioctx);
						return;
					}
				}

				break;
			}

			case IOOperation::Read:		// a write operation is complete, so post a read to get more data from the client
			{
				ioctx->recving = false;

				if (iosize == 0)	// client closed
				{
					CloseClient(ioctx);
					continue;
				}

				std::unique_ptr<Cmd> command = cmd.ParseNetCommand(std::span<std::byte>(ioctx->buffer.data(), iosize));

				if (command)
					(*command)();

				// post another read after sending
				PostRecv(ioctx);

				break;
			}

			case IOOperation::Write:	// a read operation is complete, so post a write back to the client now
				ioctx->sending = false;
				ioctx->outgoing.resize(0);
				break;
		}
	}
}

bool GetAcceptExFnPtr(const Socket &listensocket)
{
	GUID acceptexguid = WSAID_ACCEPTEX;
	DWORD bytes = 0;

	int ret = WSAIoctl(
		listensocket.GetSocket(),
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
		std::cerr << "WSAIoctl() failed to retrieve AcceptEx function address with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		return false;
	}

	return true;
}

bool PostAccept(const Socket &listensocket)
{
	std::scoped_lock lock(ioctxlistmtx);

	std::shared_ptr<IOContext> ioctx = std::make_shared<IOContext>();
	ioctx->Init(std::weak_ptr<IOContext>(ioctx));

	ioctxlist.push_back(ioctx);

	DWORD recvbytes = 0;
	int ret = AcceptExFn(
		listensocket.GetSocket(),
		ioctx->acceptsocket.GetSocket(),
		ioctx->acceptbuffer.data(),
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&recvbytes,
		&ioctx->acceptov.overlapped
	);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "AcceptEx() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		ioctxlist.remove(ioctx);
		return false;
	}

	return true;
}

void PostRecv(std::shared_ptr<IOContext> ioctx)
{
	if (ioctx->recving)
		return;

	ZeroMemory(&ioctx->recvov.overlapped, sizeof(ioctx->recvov.overlapped));

	ioctx->recvwsabuf.buf = reinterpret_cast<CHAR *>(ioctx->buffer.data());
	ioctx->recvwsabuf.len = static_cast<ULONG>(ioctx->buffer.size());

	DWORD flags = 0;
	int ret = WSARecv(ioctx->acceptsocket.GetSocket(), &ioctx->recvwsabuf, 1, nullptr, &flags, &ioctx->recvov.overlapped, nullptr);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "WSARecv() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		CloseClient(ioctx);
		return;
	}

	ioctx->recving = true;
}

void PostSend(std::shared_ptr<IOContext> ioctx)
{
	if (ioctx->sending || ioctx->outgoing.empty())
		return;

	ZeroMemory(&ioctx->sendov.overlapped, sizeof(ioctx->sendov.overlapped));

	ioctx->sendwsabuf.buf = reinterpret_cast<CHAR *>(ioctx->outgoing.data());
	ioctx->sendwsabuf.len = static_cast<ULONG>(ioctx->outgoing.size());

	int ret = WSASend(ioctx->acceptsocket.GetSocket(), &ioctx->sendwsabuf, 1, nullptr, 0, &ioctx->sendov.overlapped, nullptr);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "WSASend() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		CloseClient(ioctx);
		return;
	}

	ioctx->sending = true;
}

void CloseClient(std::shared_ptr<IOContext> ioctx)
{
	bool expected = false;
	if (!ioctx->closing.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
		return;

	std::cout << "Closing client: " << ioctx->acceptsocket.GetSocket() << std::endl;

	{
		std::scoped_lock lock(ioctxlistmtx);
		ioctxlist.remove(ioctx);
	}
}
