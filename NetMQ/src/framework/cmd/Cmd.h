#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <concepts>
#include <bit>
#include <span>
#include <tuple>

namespace CmdUtil	// functions implemented differently depending on host endianness at compile time for each type
{
	template <typename T>
	concept ValidUIntType = std::same_as<T, uint8_t>	// very cool stuff, concepts let you control instantiated typed
		|| std::same_as<T, uint16_t>
		|| std::same_as<T, uint32_t>
		|| std::same_as<T, uint64_t>;

	template<ValidUIntType T>
	constexpr std::tuple<size_t, T> ReadUInt(const std::span<const std::byte> &buffer, const size_t offset)
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
