#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstdint>
#include <cstddef>
#include <span>

namespace CmdUtil
{
	size_t ReadU32BigEndian(const std::span<const std::byte> &buffer, const size_t offset, uint32_t &out) noexcept;
}

class Cmd
{
public:
	enum class Type
	{
		Connect,
		Publish,
		Subscribe,
		Unsubscribe,
		Disconnect
	};

	virtual ~Cmd() = default;

	virtual void operator()() const = 0;
};

class ConnectCmd : public Cmd
{
protected:
	ConnectCmd(const std::span<std::byte> &params) noexcept;
	virtual ~ConnectCmd() = default;

	void operator()() const override final {}
};

class PublishCmd : public Cmd
{
protected:
	PublishCmd(const std::span<std::byte> &params) noexcept;
	virtual ~PublishCmd() = default;

	void operator()() const override final {}

	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

class SubscribeCmd : public Cmd
{
protected:
	SubscribeCmd(const std::span<std::byte> &params) noexcept;
	virtual ~SubscribeCmd() = default;

	void operator()() const override final {}

	std::span<std::byte> topic;
};

class UnsubscribeCmd : public Cmd
{
protected:
	UnsubscribeCmd(const std::span<std::byte> &params) noexcept;
	virtual ~UnsubscribeCmd() = default;

	void operator()() const override final {}

	std::span<std::byte> topic;
};

class DisconnectCmd : public Cmd
{
protected:
	DisconnectCmd(const std::span<std::byte> &params) noexcept;
	virtual ~DisconnectCmd() = default;

	void operator()() const override final {}
};

#endif
