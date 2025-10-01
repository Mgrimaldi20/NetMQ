#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstddef>
#include <span>

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
	ConnectCmd();
	virtual ~ConnectCmd();

	void operator()() const override;
};

class PublishCmd : public Cmd
{
public:
	PublishCmd(const std::span<const std::byte> &params);
	virtual ~PublishCmd();

	void operator()() const override;
};

class SubscribeCmd : public Cmd
{
public:
	SubscribeCmd(const std::span<const std::byte> &params);
	virtual ~SubscribeCmd();

	void operator()() const override;
};

class UnsubscribeCmd : public Cmd
{
public:
	UnsubscribeCmd(const std::span<const std::byte> &params);
	virtual ~UnsubscribeCmd();

	void operator()() const override;
};

class DisconnectCmd : public Cmd
{
public:
	DisconnectCmd();
	virtual ~DisconnectCmd();

	void operator()() const override;
};

#endif
