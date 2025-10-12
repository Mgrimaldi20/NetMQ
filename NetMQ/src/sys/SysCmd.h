#ifndef _NETMQ_SYSCMD_H_
#define _NETMQ_SYSCMD_H_

#include <memory>
#include <span>

#include "framework/cmd/Cmd.h"

class IOContext;

class SysCmd
{
public:
	virtual ~SysCmd() = default;

	virtual void operator()(std::shared_ptr<IOContext> ioctx) const = 0;
};

class ConnectSysCmd : public ConnectCmd, public SysCmd
{
public:
	ConnectSysCmd(const std::span<std::byte> &params) noexcept : ConnectCmd(params) {}
	virtual ~ConnectSysCmd() = default;

	void operator()(std::shared_ptr<IOContext> ioctx) const override final;
};

class PublishSysCmd : public PublishCmd, public SysCmd
{
public:
	PublishSysCmd(const std::span<std::byte> &params) noexcept : PublishCmd(params) {}
	virtual ~PublishSysCmd() = default;

	void operator()(std::shared_ptr<IOContext> ioctx) const override final;
};

class SubscribeSysCmd : public SubscribeCmd, public SysCmd
{
public:
	SubscribeSysCmd(const std::span<std::byte> &params) noexcept : SubscribeCmd(params) {}
	virtual ~SubscribeSysCmd() = default;

	void operator()(std::shared_ptr<IOContext> ioctx) const override final;
};

class UnsubscribeSysCmd : public UnsubscribeCmd, public SysCmd
{
public:
	UnsubscribeSysCmd(const std::span<std::byte> &params) noexcept : UnsubscribeCmd(params) {}
	virtual ~UnsubscribeSysCmd() = default;

	void operator()(std::shared_ptr<IOContext> ioctx) const override final;
};

class DisconnectSysCmd : public DisconnectCmd, public SysCmd
{
public:
	DisconnectSysCmd(const std::span<std::byte> &params) noexcept : DisconnectCmd(params) {}
	virtual ~DisconnectSysCmd() = default;

	void operator()(std::shared_ptr<IOContext> ioctx) const override final;
};

#endif
