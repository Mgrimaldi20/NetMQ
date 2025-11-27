#ifndef _NETMQ_OVERLAPPEDIO_H_
#define _NETMQ_OVERLAPPEDIO_H_

#include <memory>

#include "../net/WinSockAPI.h"

class IOContext;

class OverlappedIO
{
public:
	enum class Operation
	{
		Accept,
		Read,
		Write
	};

	OverlappedIO(OverlappedIO::Operation ioop, std::weak_ptr<IOContext> ioctx);
	~OverlappedIO() = default;

	void ClearOverlapped();

	WSAOVERLAPPED &GetOverlapped() noexcept;
	OverlappedIO::Operation &GetIOOperation() noexcept;

	std::shared_ptr<IOContext> GetIOContext() const noexcept;
	void SetIOContext(std::weak_ptr<IOContext> weakioctx) noexcept;

private:
	WSAOVERLAPPED overlapped;
	OverlappedIO::Operation ioop;
	std::weak_ptr<IOContext> ioctx;
};

#endif
