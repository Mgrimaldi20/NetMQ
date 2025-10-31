#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <memory>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <concepts>
#include <bit>
#include <span>
#include <tuple>

#include "framework/Bitmask.h"

class IOContext;

namespace CmdUtil	// functions implemented differently depending on host endianness at compile time for each type
{
	template <typename T>
	concept ValidUIntType = std::same_as<T, uint8_t>	// very cool stuff, concepts let you control instantiated typed
		|| std::same_as<T, uint16_t>
		|| std::same_as<T, uint32_t>
		|| std::same_as<T, uint64_t>;

	template<ValidUIntType T>
	constexpr std::tuple<size_t, T> ReadUInt(std::span<const std::byte> buffer, const size_t offset)
	{
		static constexpr size_t OUT_SIZE = sizeof(T);

		T out = 0;

		if (buffer.size() < (offset + OUT_SIZE))
			return std::make_tuple(0, out);

		if constexpr (std::same_as<T, uint8_t>)
			out = std::to_integer<uint8_t>(buffer[offset]);

		else
		{
			std::memcpy(&out, buffer.data() + offset, OUT_SIZE);

			if constexpr (std::endian::native == std::endian::little)
				out = std::byteswap(out);
		}

		return std::make_tuple(OUT_SIZE, out);
	}
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
		Disconnect,
		Ping
	};

	Cmd(std::shared_ptr<IOContext> ioctx) noexcept
		: ioctx(ioctx)
	{}

	virtual ~Cmd() = default;

	virtual void operator()() const = 0;

protected:
	std::shared_ptr<IOContext> ioctx;
};

class ConnectCmd : public Cmd
{
public:
	enum class Flags : uint16_t;

	ConnectCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~ConnectCmd() = default;

	void operator()() const override final;

private:
	Flags flags;

	std::span<std::byte> clientid;
};

template<>
struct Bitmask::EnableBitmaskOperators<ConnectCmd::Flags> : std::true_type {};

enum class ConnectCmd::Flags : uint16_t
{
	ClientId = Bitmask::Bit<Flags, 0>(),
	AuthTkn = Bitmask::Bit<Flags, 1>()
};

class PublishCmd : public Cmd
{
public:
	PublishCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~PublishCmd() = default;

	void operator()() const override final;

private:
	std::span<std::byte> topic;
	std::span<std::byte> msg;
};

class SubscribeCmd : public Cmd
{
public:
	enum class Flags : uint16_t;

	SubscribeCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~SubscribeCmd() = default;

	void operator()() const override final;

private:
	Flags flags;

	std::span<std::byte> topic;
};

template<>
struct Bitmask::EnableBitmaskOperators<SubscribeCmd::Flags> : std::true_type {};

enum class SubscribeCmd::Flags : uint16_t
{
	None = 0,
	PostAllData = Bitmask::Bit<Flags, 0>(),
	PostChangedData = Bitmask::Bit<Flags, 1>()
};

class UnsubscribeCmd : public Cmd
{
public:
	UnsubscribeCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~UnsubscribeCmd() = default;

	void operator()() const override final;

private:
	std::span<std::byte> topic;
};

class DisconnectCmd : public Cmd
{
public:
	DisconnectCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~DisconnectCmd() = default;

	void operator()() const override final;
};

class PingCmd : public Cmd
{
public:
	PingCmd(std::shared_ptr<IOContext> ioctx, std::span<std::byte> params);
	virtual ~PingCmd() = default;

	void operator()() const override final;
};

#endif
