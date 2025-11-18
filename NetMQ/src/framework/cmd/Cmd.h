#ifndef _NETMQ_CMD_H_
#define _NETMQ_CMD_H_

#include <cstdint>
#include <cstddef>
#include <utility>
#include <concepts>
#include <memory>
#include <bit>
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
}

class Cmd
{
public:
	enum class Type
	{
		Ping,
		Connect,
		Publish,
		Subscribe,
		Unsubscribe,
		Disconnect
	};

	virtual ~Cmd() = default;

	void operator()() const;

protected:
	Cmd(std::shared_ptr<IOContext> ioctx, SubManager &manager) noexcept;
	
	virtual void ExecuteCmd() const = 0;
	virtual void ExecuteAck() const = 0;

	virtual const bool AckRequired() const noexcept;

	std::shared_ptr<IOContext> ioctx;

	SubManager &manager;

	friend class CmdSystem;
};

#endif
