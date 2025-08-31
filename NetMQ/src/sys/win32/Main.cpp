#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "WinSockAPI.h"

#include <iostream>
#include <system_error>
#include <string>
#include <format>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <unordered_set>
#include <list>

#include "framework/Log.h"
#include "framework/Cmd.h"

static constexpr unsigned short NET_DEFAULT_PORT = 5001;
static constexpr unsigned int NET_DEFAULT_THREADS = 2;
static constexpr size_t NET_MAX_BUFFER_SIZE = 8192;

struct IOContext;

enum class IOOperation
{
	Accept,
	Read,
	Write
};

struct OverlappedIO
{
	OverlappedIO(IOOperation ioop, IOContext *ioctx)
		: ioop(ioop),
		ioctx(ioctx)
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	WSAOVERLAPPED overlapped;
	IOOperation ioop;
	IOContext *ioctx;
};

struct IOContext
{
	IOContext()
		: acceptsocket(INVALID_SOCKET),
		acceptov(IOOperation::Accept, this),
		recvov(IOOperation::Read, this),
		sendov(IOOperation::Write, this),
		recvwsabuf(),
		sendwsabuf(),
		recving(false),
		sending(false),
		buffer(),
		subscriptions(),
		outgoing(),
		iter()
	{
	}

	SOCKET acceptsocket;
	OverlappedIO acceptov;
	OverlappedIO recvov;
	OverlappedIO sendov;
	WSABUF recvwsabuf;
	WSABUF sendwsabuf;
	std::atomic<bool> recving;
	std::atomic<bool> sending;
	char buffer[NET_MAX_BUFFER_SIZE];
	std::unordered_set<std::string> subscriptions;
	std::string outgoing;
	std::list<IOContext>::iterator iter;
};

const std::string GetErrorMessage(const DWORD errcode);
BOOL WINAPI CtrlHandler(DWORD event);

void WorkerThread(HANDLE &iocp, SOCKET &listensocket, CmdSystem &cmd, Log &log);

SOCKET CreateSocket();
bool CreateListenSocket(const SOCKET &listensocket);
bool CreateAcceptSocket(const SOCKET &listensocket, const HANDLE &iocp, const bool updateiocp);
void PostRecv(IOContext &ioctx);
void PostSend(IOContext &ioctx);
void CloseClient(IOContext *ioctx);

LPFN_ACCEPTEX AcceptExFn;

std::list<IOContext> ioctxlist;
std::mutex ioctxlistmtx;

std::atomic<bool> endserver;
std::atomic<bool> restartserver;
std::condition_variable cleanupcv;

void Publish_Cmd(const std::any &userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if ((argc < 3) || (argc > 3))
	{
		std::cout << "Usage: " << args[0] << " [topic] [value]" << std::endl;
		return;
	}

	(const std::any &)userdata;

	std::scoped_lock lock(ioctxlistmtx);

	for (IOContext &ioctx : ioctxlist)
	{
		if (ioctx.acceptsocket == INVALID_SOCKET || !ioctx.subscriptions.contains(args[1]) || ioctx.sending)
			continue;

		ioctx.outgoing = args[2];

		PostSend(ioctx);
	}
}

void Subscribe_Cmd(const std::any &userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if ((argc < 2) || (argc > 2))
	{
		std::cout << "Usage: " << args[0] << " [topic]" << std::endl;
		return;
	}

	IOContext *ioctx = std::any_cast<IOContext *>(userdata);

	ioctx->subscriptions.insert(args[1]);
}

void Unsubscribe_Cmd(const std::any &userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if ((argc < 2) || (argc > 2))
	{
		std::cout << "Usage: " << args[0] << " [topic]" << std::endl;
		return;
	}

	IOContext *ioctx = std::any_cast<IOContext *>(userdata);

	ioctx->subscriptions.erase(args[1]);
}

void Exit_Cmd(const std::any &userdata, const CmdArgs &args)
{
	const size_t argc = args.GetCount();
	if (argc > 1)
	{
		std::cout << "Usage: " << args[0] << std::endl;
		return;
	}

	IOContext *ioctx = std::any_cast<IOContext *>(userdata);

	CloseClient(ioctx);
}

int main(int argc, char **argv)
{
	(int)argc;
	(char **)argv;

	Log log;
	CmdSystem cmd(log);

	cmd.RegisterCommand("pub", Publish_Cmd, "Publishes data to a specified topic, best effort");
	cmd.RegisterCommand("sub", Subscribe_Cmd, "Subscribes to a topic to get incoming data from publisher");
	cmd.RegisterCommand("unsub", Unsubscribe_Cmd, "Unsubscribe from a topic to stop receiving its data");
	cmd.RegisterCommand("exit", Exit_Cmd, "Disconnect the client from the server entirely");

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

		HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (!iocp)
		{
			log.Error("CreateIoCompletionPort() failed with error (initial): {}", GetErrorMessage(GetLastError()));
			return 1;
		}

		SOCKET listensocket = CreateSocket();
		if (listensocket == INVALID_SOCKET)
		{
			log.Error("CreateSocket() failed");
			CloseHandle(iocp);
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		if (!CreateListenSocket(listensocket))
		{
			log.Error("CreateListenSocket() failed");
			closesocket(listensocket);
			CloseHandle(iocp);
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		if (!CreateAcceptSocket(listensocket, iocp, true))
		{
			log.Error("CreateListenSocket() failed");
			closesocket(listensocket);
			CloseHandle(iocp);
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
		log.Info("Echo server running on port: {}", NET_DEFAULT_PORT);
		log.Info("Press Ctrl-C to exit, or Ctrl-Break to restart...");

		{
			std::mutex cvmtx;
			std::unique_lock lock(cvmtx);
			cleanupcv.wait(lock, [] { return endserver.load(); });
		}

		endserver = true;

		if (iocp)
		{
			for (unsigned int i=0; i<numthreads; i++)
				PostQueuedCompletionStatus(iocp, 0, 0, nullptr);
		}

		for (std::thread &thread : threads)
		{
			if (thread.joinable())
				thread.join();
		}

		if (listensocket != INVALID_SOCKET)
		{
			closesocket(listensocket);		// stop accepting new connections and cause any accepts to fail
			listensocket = INVALID_SOCKET;
		}

		{
			std::scoped_lock lock(ioctxlistmtx);

			for (IOContext &ioctx : ioctxlist)
			{
				if (ioctx.acceptsocket == INVALID_SOCKET)
				{
					log.Warn("Socket context is alread INVALID");
					continue;
				}

				log.Info("Closing client: {}", ioctx.acceptsocket);

				shutdown(ioctx.acceptsocket, SD_BOTH);
				CancelIoEx((HANDLE)ioctx.acceptsocket, &ioctx.acceptov.overlapped);
				CancelIoEx((HANDLE)ioctx.acceptsocket, &ioctx.sendov.overlapped);
				CancelIoEx((HANDLE)ioctx.acceptsocket, &ioctx.recvov.overlapped);

				closesocket(ioctx.acceptsocket);
				ioctx.acceptsocket = INVALID_SOCKET;
			}

			ioctxlist.clear();
		}

		if (iocp)
		{
			CloseHandle(iocp);
			iocp = nullptr;
		}

		if (restartserver.load())
			log.Info("NetMQ is restarting...\n");

		else
			log.Info("NetMQ is exiting...");
	}

	SetConsoleCtrlHandler(CtrlHandler, FALSE);

	return 0;
}

const std::string GetErrorMessage(const DWORD errcode)
{
	std::error_condition errcond = std::system_category().default_error_condition(errcode);

	return std::format(
		"[Category: {}, Value: {}, Message: {}]",
		errcond.category().name(),
		errcond.value(),
		errcond.message()
	);
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

void WorkerThread(HANDLE &iocp, SOCKET &listensocket, CmdSystem &cmd, Log &log)
{
	DWORD iosize = 0;
	ULONG_PTR completionkey = 0;
	WSAOVERLAPPED *wsaoverlapped = nullptr;

	while (true)
	{
		bool status = GetQueuedCompletionStatus(iocp, &iosize, &completionkey, &wsaoverlapped, INFINITE);
		if (!status)
			log.Error("GetQueuedCompletionStatus() failed with error: {}", GetErrorMessage(GetLastError()));

		if ((!completionkey && !wsaoverlapped) || (!status && !wsaoverlapped))
			break;

		if (!wsaoverlapped)
			continue;

		OverlappedIO *perio = reinterpret_cast<OverlappedIO *>(wsaoverlapped);
		IOContext *ioctx = perio->ioctx;

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
				int ret = setsockopt(ioctx->acceptsocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&listensocket, sizeof(listensocket));
				if (ret == SOCKET_ERROR)
				{
					log.Error("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket: {}", GetErrorMessage(WSAGetLastError()));
					CloseClient(ioctx);
					return;
				}

				if (!CreateIoCompletionPort((HANDLE)ioctx->acceptsocket, iocp, (ULONG_PTR)ioctx->acceptsocket, 0))
				{
					log.Error("CreateIoCompletionPort() failed to associate iocp handle to accept socket: {}", GetErrorMessage(GetLastError()));
					CloseClient(ioctx);
					return;
				}

				// start a read from a new client
				PostRecv(*ioctx);

				// post another accept for a new client if the server isnt stopping
				if (!endserver.load())
				{
					if (!CreateAcceptSocket(listensocket, iocp, false))
					{
						log.Error("CreateAcceptSocket() failed to post an accept for a new client");
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

				size_t recvsize = (iosize < NET_MAX_BUFFER_SIZE) ? iosize : (NET_MAX_BUFFER_SIZE - 1);
				ioctx->buffer[recvsize] = '\0';

				cmd.ExecuteCommand(ioctx, ioctx->buffer);

				// post another read after sending
				PostRecv(*ioctx);

				break;
			}

			case IOOperation::Write:	// a read operation is complete, so post a write back to the client now
				ioctx->sending = false;
				ioctx->outgoing.clear();
				break;
		}
	}
};

SOCKET CreateSocket()
{
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
	{
		std::cerr << "WSASocket() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		return socket;
	}

	int zero = 0;
	int ret = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "setsockopt(SO_SNDBUF) failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
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
		std::cerr << "bind() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		return false;
	}

	ret = listen(listensocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "listen() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		return false;
	}

	return true;
}

bool CreateAcceptSocket(const SOCKET &listensocket, const HANDLE &iocp, const bool updateiocp)
{
	std::scoped_lock lock(ioctxlistmtx);

	if (updateiocp)
	{
		if (!CreateIoCompletionPort((HANDLE)listensocket, iocp, 0, 0))
		{
			std::cerr << "CreateIoCompletionPort() failed to associate iocp handle to listen socket: " << GetErrorMessage(GetLastError()) << std::endl;
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
			std::cerr << "WSAIoctl() failed to retrieve AcceptEx function address with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
			return false;
		}
	}

	std::list<IOContext>::iterator iter = ioctxlist.emplace(ioctxlist.end());

	IOContext &ioctx = *iter;
	ioctx.iter = iter;

	ioctx.acceptsocket = CreateSocket();
	if (ioctx.acceptsocket == INVALID_SOCKET)
	{
		std::cerr << "CreateAcceptSocket() failed" << std::endl;
		ioctxlist.erase(iter);
		return false;
	}

	DWORD recvbytes = 0;
	int ret = AcceptExFn(
		listensocket,
		ioctx.acceptsocket,
		ioctx.buffer,
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&recvbytes,
		&ioctx.acceptov.overlapped
	);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "AcceptEx() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		closesocket(ioctx.acceptsocket);
		ioctxlist.erase(iter);
		return false;
	}

	return true;
}

void PostRecv(IOContext &ioctx)
{
	if (ioctx.recving || ioctx.acceptsocket == INVALID_SOCKET)
		return;

	ZeroMemory(&ioctx.recvov.overlapped, sizeof(ioctx.recvov.overlapped));

	ioctx.recvwsabuf.buf = ioctx.buffer;
	ioctx.recvwsabuf.len = NET_MAX_BUFFER_SIZE;

	DWORD flags = 0;
	int ret = WSARecv(ioctx.acceptsocket, &ioctx.recvwsabuf, 1, nullptr, &flags, &ioctx.recvov.overlapped, nullptr);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "WSARecv() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		CloseClient(&ioctx);
		return;
	}

	ioctx.recving = true;
}

void PostSend(IOContext &ioctx)
{
	if (ioctx.sending || ioctx.acceptsocket == INVALID_SOCKET || ioctx.outgoing.empty())
		return;

	ZeroMemory(&ioctx.sendov.overlapped, sizeof(ioctx.sendov.overlapped));

	ioctx.sendwsabuf.buf = ioctx.outgoing.data();
	ioctx.sendwsabuf.len = static_cast<ULONG>(ioctx.outgoing.size());

	int ret = WSASend(ioctx.acceptsocket, &ioctx.sendwsabuf, 1, nullptr, 0, &ioctx.sendov.overlapped, nullptr);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		std::cerr << "WSASend() failed with error: " << GetErrorMessage(WSAGetLastError()) << std::endl;
		CloseClient(&ioctx);
		return;
	}

	ioctx.sending = true;
}

void CloseClient(IOContext *ioctx)
{
	std::scoped_lock lock(ioctxlistmtx);

	if (!ioctx)
		return;

	if (ioctx->acceptsocket != INVALID_SOCKET)
	{
		std::cout << "Closing client: " << ioctx->acceptsocket << std::endl;
		closesocket(ioctx->acceptsocket);
		ioctx->acceptsocket = INVALID_SOCKET;
	}

	ioctxlist.erase(ioctx->iter);
}
