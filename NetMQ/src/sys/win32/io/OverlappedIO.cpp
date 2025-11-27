#include "OverlappedIO.h"

OverlappedIO::OverlappedIO(OverlappedIO::Operation ioop, std::weak_ptr<IOContext> ioctx)
	: ioop(ioop),
	ioctx(ioctx)
{
	ClearOverlapped();
}

void OverlappedIO::ClearOverlapped()
{
	ZeroMemory(&overlapped, sizeof(overlapped));
}

WSAOVERLAPPED &OverlappedIO::GetOverlapped() noexcept
{
	return overlapped;
}

OverlappedIO::Operation &OverlappedIO::GetIOOperation() noexcept
{
	return ioop;
}

std::shared_ptr<IOContext> OverlappedIO::GetIOContext() const noexcept
{
	return ioctx.lock();
}

void OverlappedIO::SetIOContext(std::weak_ptr<IOContext> weakioctx) noexcept
{
	ioctx = weakioctx;
}
