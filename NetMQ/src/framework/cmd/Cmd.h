#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstdint>
#include <cstddef>
#include <utility>
#include <concepts>
#include <type_traits>
#include <memory>
#include <bit>
#include <vector>
#include <span>

#include "framework/SubManager.h"

namespace CmdUtil	// functions implemented differently depending on host endianness at compile time for each type
{
	template<typename T>
	concept ValidUIntType = std::same_as<T, uint8_t>	// very cool stuff, concepts let you control instantiated typed
		|| std::same_as<T, uint16_t>
		|| std::same_as<T, uint32_t>
		|| std::same_as<T, uint64_t>;

	template<ValidUIntType T>
	constexpr std::pair<size_t, T> ReadUInt(std::span<const std::byte> buffer, const size_t offset)
	{
		static constexpr size_t OUT_SIZE = sizeof(T);

		T out = 0;

		if (buffer.size() < (offset + OUT_SIZE))
			return std::make_pair(0, out);

		if constexpr (std::same_as<T, uint8_t>)
			out = std::to_integer<uint8_t>(buffer[offset]);

		else
		{
			std::memcpy(&out, buffer.data() + offset, OUT_SIZE);

			if constexpr (std::endian::native == std::endian::little)
				out = std::byteswap(out);
		}

		return std::make_pair(OUT_SIZE, out);
	}

	class AckBuilder
	{
	public:
		AckBuilder()
			: buffer()
		{
		}

		~AckBuilder() = default;

		template<ValidUIntType T>
		AckBuilder &AppendUInt(const T val)		// append an unsigned int, go back to little endian if needed
		{
			T out = val;

			if constexpr(std::same_as<T, uint8_t>)
				buffer.push_back(std::byte(out));

			else
			{
				if constexpr (std::endian::native == std::endian::little)
					out = std::byteswap(out);

				buffer.assign_range(std::as_bytes(std::span<const T>(&out, 1)));
			}

			return *this;
		}

		AckBuilder &AppendString(std::string_view string)
		{
			for (char c : string)
				buffer.push_back(std::byte(c));

			return *this;
		}

		AckBuilder &AppendBytes(std::span<const std::byte> bytes)
		{
			buffer.assign_range(bytes);
			return *this;
		}

		std::span<const std::byte> Build() const
		{
			return std::span<const std::byte>(buffer);
		}

	private:
		std::vector<std::byte> buffer;
	};
}

class Cmd
{
public:
	enum class Type : uint8_t
	{
		Ping,
		Connect,
		Publish,
		Subscribe,
		Unsubscribe,
		Disconnect
	};

	virtual ~Cmd() = default;

	void operator()();

protected:
	Cmd(std::shared_ptr<IOContext> ioctx, SubManager &manager) noexcept;

	CmdUtil::AckBuilder ackbuilder;

	std::shared_ptr<IOContext> ioctx;
	SubManager &manager;

private:
	virtual void ExecuteCmd() = 0;
	virtual void ExecuteAck() = 0;

	virtual const bool AckRequired() const noexcept;
};

enum class ReasonCode : uint8_t
{
	Success
};

#endif
