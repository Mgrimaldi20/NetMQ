#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstdint>
#include <cstddef>
#include <span>

namespace CmdUtil
{
	size_t ReadU32BigEndian(const std::span<const std::byte> const &buffer, const size_t offset, uint32_t &out) noexcept;
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

	virtual void operator()() const = 0;
};

class ConnectCmd : public Cmd
{
public:
	ConnectCmd() noexcept;
	virtual ~ConnectCmd() {}

	void operator()() const override;
};

class PublishCmd : public Cmd
{
public:
	PublishCmd(const std::span<std::byte> &params) noexcept;
	virtual ~PublishCmd() {}

	void operator()() const override;

private:
	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

class SubscribeCmd : public Cmd
{
public:
	SubscribeCmd(const std::span<std::byte> &params) noexcept;
	virtual ~SubscribeCmd() {}

	void operator()() const override;

private:
	std::span<std::byte> topic;
};

class UnsubscribeCmd : public Cmd
{
public:
	UnsubscribeCmd(const std::span<std::byte> &params) noexcept;
	virtual ~UnsubscribeCmd() {}

	void operator()() const override;

private:
	std::span<std::byte> topic;
};

class DisconnectCmd : public Cmd
{
public:
	DisconnectCmd() noexcept;
	virtual ~DisconnectCmd() {}

	void operator()() const override;
};

#endif
