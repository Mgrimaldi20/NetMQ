#include "IOContext.h"

const std::string GetErrorMessage(const int errcode);

IOContext::IOContext(Log &log, std::list<std::shared_ptr<IOContext>> &ioctxlist, std::mutex &ioctxlistmtx)
	: acceptsocket(),
	clientid(),
	acceptov(IOOperation::Accept, {}),
	recvov(IOOperation::Read, {}),
	sendov(IOOperation::Write, {}),
	recvwsabuf(),
	sendwsabuf(),
	timeout(5000),
	iocv(),
	iomtx(),
	connected(false),
	recving(false),
	sending(false),
	buffer(NET_MAX_BUFFER_SIZE),
	outgoing(),
	acceptbuffer((sizeof(SOCKADDR_STORAGE) + 16) * 2),
	iter(),
	closing(false),
	log(log),
	ioctxlist(ioctxlist),
	ioctxlistmtx(ioctxlistmtx)
{
	outgoing.reserve(100);	// this should be configurable based on the workload
}

IOContext::~IOContext()
{
	CancelOverlappedIO();
}

void IOContext::Initialize()
{
	std::shared_ptr<IOContext> self = shared_from_this();
	acceptov.SetIOContext(self);
	recvov.SetIOContext(self);
	sendov.SetIOContext(self);
}

void IOContext::SetListIter(std::list<std::shared_ptr<IOContext>>::iterator listiter) noexcept
{
	iter = listiter;
}

void IOContext::PostRecv()
{
	if (recving.load() || closing.load(std::memory_order_acquire))
		return;

	recvov.ClearOverlapped();

	recvwsabuf.buf = reinterpret_cast<CHAR *>(buffer.data());
	recvwsabuf.len = static_cast<ULONG>(buffer.size());

	int ret = acceptsocket.Recv(recvwsabuf, recvov);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		log.Error("WSARecv() failed with error: {}", GetErrorMessage(WSAGetLastError()));
		CloseClient();
		return;
	}

	SetRecving(true);
}

void IOContext::PostSend(std::span<const std::byte> data)
{
	if (sending.load() || data.empty() || closing.load(std::memory_order_acquire))
		return;

	outgoing.assign_range(data);

	sendov.ClearOverlapped();

	sendwsabuf.buf = reinterpret_cast<CHAR *>(outgoing.data());
	sendwsabuf.len = static_cast<ULONG>(outgoing.size());

	int ret = acceptsocket.Send(sendwsabuf, sendov);
	if (ret == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		log.Error("WSASend() failed with error: {}", GetErrorMessage(WSAGetLastError()));
		CloseClient();
		return;
	}

	SetSending(true);
}

void IOContext::CloseClient()
{
	bool expected = false;
	if (!closing.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
		return;

	std::shared_ptr<IOContext> self = shared_from_this();

	{
		std::unique_lock<std::mutex> lock(iomtx);

		auto pred = [this]() noexcept
		{
			return !recving.load(std::memory_order_acquire) && !sending.load(std::memory_order_acquire);
		};

		if (!iocv.wait_for(lock, timeout, pred))
			log.Warn("CloseClient() timeout waiting for IO operations to complete for client: {}", clientid);
	}

	log.Info("Closing client: {}", clientid);

	{
		std::scoped_lock lock(ioctxlistmtx);
		ioctxlist.erase(iter);
	}
}

void IOContext::CancelOverlappedIO() noexcept
{
	acceptsocket.CancelIO(acceptov);
	acceptsocket.CancelIO(sendov);
	acceptsocket.CancelIO(recvov);
}

Socket &IOContext::GetAcceptSocket() noexcept
{
	return acceptsocket;
}

void IOContext::SetClientID(const std::string &id) noexcept
{
	clientid = id;
}

OverlappedIO &IOContext::GetAcceptOverlapped() noexcept
{
	return acceptov;
}

std::atomic<bool> &IOContext::GetConnected() noexcept
{
	return connected;
}

void IOContext::SetConnected(bool val) noexcept
{
	connected = val;
}

std::atomic<bool> &IOContext::GetRecving() noexcept
{
	return recving;
}

void IOContext::SetRecving(bool val) noexcept
{
	recving.store(val, std::memory_order_release);
	if (!val)
	{
		std::scoped_lock lock(iomtx);
		iocv.notify_all();
	}
}

std::atomic<bool> &IOContext::GetSending() noexcept
{
	return sending;
}

void IOContext::SetSending(bool val) noexcept
{
	sending.store(val, std::memory_order_release);
	if (!val)
	{
		std::scoped_lock lock(iomtx);
		iocv.notify_all();
	}
}

std::vector<std::byte> &IOContext::GetIncomingBuffer() noexcept
{
	return buffer;
}

std::vector<std::byte> &IOContext::GetAcceptBuffer() noexcept
{
	return acceptbuffer;
}
