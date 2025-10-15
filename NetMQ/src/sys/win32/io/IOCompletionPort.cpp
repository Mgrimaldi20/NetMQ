#include <stdexcept>
#include <format>

#include "IOCompletionPort.h"

const std::string GetErrorMessage(const int errcode);

IOCompletionPort::IOCompletionPort()
	: iocp(INVALID_HANDLE_VALUE)
{
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (!iocp)
		throw std::runtime_error(std::format("CreateIoCompletionPort() failed with error (initial): {}", GetErrorMessage(GetLastError())));
}

IOCompletionPort::~IOCompletionPort()
{
	if (iocp && iocp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(iocp);
		iocp = INVALID_HANDLE_VALUE;
	}
}

bool IOCompletionPort::UpdateIOCompletionPort(Socket &socket, ULONG_PTR completionkey) const
{
	if (!CreateIoCompletionPort((HANDLE)socket.GetSocket(), iocp, completionkey, 0))
		return false;

	return true;
}

bool IOCompletionPort::GetQueuedCompletionStatus(unsigned long *iosize, unsigned long long *completionkey, WSAOVERLAPPED **wsaoverlapped) const
{
	return ::GetQueuedCompletionStatus(iocp, iosize, completionkey, wsaoverlapped, INFINITE);
}

bool IOCompletionPort::PostQueuedQuitStatus()
{
	return PostQueuedCompletionStatus(iocp, 0, 0, nullptr);
}
