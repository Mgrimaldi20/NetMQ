#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "net/WinSockAPI.h"

#include <iostream>

#include "framework/Log.h"
#include "framework/CmdSystem.h"

#include "net/Socket.h"
#include "io/IOContext.h"
#include "io/IOCompletionPort.h"

#include "../Cmd.h"

constexpr unsigned int NET_DEFAULT_THREADS = 2;
constexpr unsigned int NET_DEFAULT_PRE_POST_ACCEPTS = 10;

const std::string GetErrorMessage(const int errcode);
bool ValidateOptions(int argc, char **argv);
BOOL WINAPI CtrlHandler(DWORD event);

void WorkerThread(const IOCompletionPort &iocp, Socket &listensocket, const CmdSystem &cmd, Log &log);
bool GetAcceptExFnPtr(Socket &listensocket, Log &log);
bool PostAccept(Socket &listensocket, Log &log);

LPFN_ACCEPTEX AcceptExFn;

std::list<std::shared_ptr<IOContext>> ioctxlist;
std::mutex ioctxlistmtx;

std::atomic<bool> endserver;
std::atomic<bool> restartserver;
std::condition_variable cleanupcv;

static std::string_view serverport = Socket::NET_DEFAULT_PORT;

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

		listensocket.Bind(serverport);
		listensocket.Listen();

		if (!iocp.UpdateIOCompletionPort(listensocket, 0))
		{
			log.Error("UpdateIOCompletionPort() failed to associate iocp handle to listen socket: {}", GetErrorMessage(GetLastError()));
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		if (!GetAcceptExFnPtr(listensocket, log))
		{
			log.Error("GetAcceptExFnPtr() failed");
			SetConsoleCtrlHandler(CtrlHandler, FALSE);
			return 1;
		}

		for (unsigned int i=0; i<NET_DEFAULT_PRE_POST_ACCEPTS; i++)
		{
			if (!PostAccept(listensocket, log))
			{
				log.Error("PostAccept() failed (initial)");
				SetConsoleCtrlHandler(CtrlHandler, FALSE);
				return 1;
			}
		}

		unsigned int numthreads = std::thread::hardware_concurrency() * 2;
		numthreads = (numthreads == 0) ? NET_DEFAULT_THREADS : numthreads;

		if (numthreads == NET_DEFAULT_THREADS)
		{
			log.Warn("The number of threads avaliable is equal to the default number [{}]", NET_DEFAULT_THREADS);
			log.Warn("If this is not correct, you may wish to restart NetMQ as the correct number of system threads have not been detected");
		}

		std::vector<std::thread> threads;
		for (unsigned int i=0; i<numthreads; i++)
			threads.emplace_back(WorkerThread, std::ref(iocp), std::ref(listensocket), std::ref(cmd), std::ref(log));

		log.Info("Number of threads available: {}", numthreads);
		log.Info("NetMQ server running on port: {}", serverport);
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
			{
				std::string_view arg(argv[i]);

				if (arg.length() > 3 && arg[2] == ':')
					serverport = arg.substr(3);

				break;
			}

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

void WorkerThread(const IOCompletionPort &iocp, Socket &listensocket, const CmdSystem &cmd, Log &log)
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

		switch (perio->GetIOOperation())
		{
			case IOOperation::Accept:
			{
				// after AcceptEx completed
				SOCKET ls = listensocket.GetSocket();
				int ret = setsockopt(ioctx->GetAcceptSocket().GetSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char *>(&ls), sizeof(ls));
				if (ret == SOCKET_ERROR)
				{
					log.Error("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed to update accept socket: {}", GetErrorMessage(WSAGetLastError()));
					ioctx->CloseClient();
					return;
				}

				if (!iocp.UpdateIOCompletionPort(ioctx->GetAcceptSocket(), static_cast<ULONG_PTR>(ioctx->GetAcceptSocket().GetSocket())))
				{
					log.Error("UpdateIOCompletionPort() failed to associate iocp handle to accept socket: {}", GetErrorMessage(GetLastError()));
					ioctx->CloseClient();
					return;
				}

				// start a read from a new client
				ioctx->PostRecv();

				// post another accept for a new client if the server isnt stopping
				if (!endserver.load())
				{
					if (!PostAccept(listensocket, log))
					{
						log.Error("PostAccept() failed to post an accept for a new client");
						ioctx->CloseClient();
						return;
					}
				}

				break;
			}

			case IOOperation::Read:		// a write operation is complete, so post a read to get more data from the client
			{
				ioctx->SetRecving(false);

				if (iosize == 0)	// client closed
				{
					ioctx->CloseClient();
					continue;
				}

				std::unique_ptr<Cmd> command = cmd.ParseCommand(ioctx, std::span<std::byte>(ioctx->GetIncomingBuffer().data(), iosize));
				if (command)
					(*command)();

				// post another read after sending
				ioctx->PostRecv();

				break;
			}

			case IOOperation::Write:	// a read operation is complete, so post a write back to the client now
				ioctx->SetSending(false);
				ioctx->GetOutgoingBuffer().resize(0);
				break;
		}
	}
}

bool GetAcceptExFnPtr(Socket &listensocket, Log &log)
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
		log.Error("WSAIoctl() failed to retrieve AcceptEx function address with error: {}", GetErrorMessage(WSAGetLastError()));
		return false;
	}

	return true;
}

bool PostAccept(Socket &listensocket, Log &log)
{
	std::scoped_lock lock(ioctxlistmtx);

	std::list<std::shared_ptr<IOContext>>::iterator iter = ioctxlist.emplace(ioctxlist.end(), std::make_shared<IOContext>(log, ioctxlist, ioctxlistmtx));

	std::shared_ptr<IOContext> &ioctx = *iter;
	ioctx->Initialize();
	ioctx->SetListIter(iter);

	DWORD recvbytes = 0;
	int ret = AcceptExFn(
		listensocket.GetSocket(),
		ioctx->GetAcceptSocket().GetSocket(),
		ioctx->GetAcceptBuffer().data(),
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&recvbytes,
		&ioctx->GetAcceptOverlapped().GetOverlapped()
	);

	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		log.Error("AcceptEx() failed with error: {}", GetErrorMessage(WSAGetLastError()));
		ioctx->CloseClient();
		return false;
	}

	return true;
}
