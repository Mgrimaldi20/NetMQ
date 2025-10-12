#ifndef _NETMQ_OVERLAPPEDIO_H_
#define _NETMQ_OVERLAPPEDIO_H_

#include <memory>

#include "../net/WinSockAPI.h"

#include "IOOperation.h"

class IOContext;

class OverlappedIO
{
public:
	OverlappedIO(IOOperation ioop, std::weak_ptr<IOContext> ioctx);
	~OverlappedIO() = default;

	void ClearOverlapped();

	WSAOVERLAPPED &GetOverlapped() noexcept;
	IOOperation &GetIOOperation() noexcept;

	std::shared_ptr<IOContext> GetIOContext() const noexcept;
	void SetIOContext(std::weak_ptr<IOContext> weakioctx) noexcept;

private:
	WSAOVERLAPPED overlapped;
	IOOperation ioop;
	std::weak_ptr<IOContext> ioctx;
};

#endif
