#ifndef _NETMQ_BYTEBUFFER_H_
#define _NETMQ_BYTEBUFFER_H_

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <bit>
#include <iterator>
#include <vector>
#include <span>
#include <string_view>
#include <string>

namespace
{
	template<typename T>
	concept ValidUIntType = std::same_as<T, uint8_t>	// very cool stuff, concepts let you control instantiated types
		|| std::same_as<T, uint16_t>
		|| std::same_as<T, uint32_t>
		|| std::same_as<T, uint64_t>;

	template<typename T>
	concept ValidUIntUnderlyingType = std::is_enum_v<T>
		&& ValidUIntType<std::underlying_type_t<T>>;
}

/*
* Class: ByteBuffer
* An abstract data buffer for reading and writing various data types to and from a byte vector.
* functions implemented differently depending on host endianness at compile time for each type.
* Meant for use in network communication protocols, incoming data is expected to be in big-endian format.
* Variable ints sre using the MQTT Variable Byte Integer encoding/decoding scheme.
*
*	ReadUInt<T>: Reads an unsigned integer of type T including enum underlying types
*	ReadVarUInt: Reads a variable length unsigned integer using MQTT Variable Byte Integer encoding
* 	ReadString: Reads a string of length L from the buffer
*	ReadBytes: Reads a byte span of length L from the buffer
*	WriteUInt<T>: Writes an unsigned integer of type T including enum underlying types
*	WriteVarUInt: Writes a variable length unsigned integer using MQTT Variable Byte Integer encoding
*	WriteString: Writes a string to the buffer
*	WriteBytes: Writes a byte span to the buffer
*	Build: Builds and returns a span of the internal byte buffer
*	Data: Returns a reference to the internal byte buffer data
*	Size: Returns the size of the internal byte buffer
*	Empty: Returns if the internal byte buffer is empty
*/
class ByteBuffer
{
public:
	std::byte &operator[](size_t index) noexcept;

	ByteBuffer() = default;
	ByteBuffer(const ByteBuffer &) = default;
	ByteBuffer(ByteBuffer &&) noexcept = default;
	ByteBuffer(std::span<std::byte> buf);
	ByteBuffer(std::vector<std::byte> buf);
	~ByteBuffer() = default;

	template<ValidUIntType T>
	inline constexpr T ReadUInt();

	template<ValidUIntUnderlyingType T>
	inline constexpr T ReadUInt();

	constexpr uintmax_t ReadVarUInt();
	constexpr std::string ReadString(size_t length);
	constexpr std::span<const std::byte> ReadBytes(size_t length);

	template<ValidUIntType T>
	inline ByteBuffer &WriteUInt(const T val);

	template<ValidUIntUnderlyingType T>
	inline ByteBuffer &WriteUInt(const T val);

	ByteBuffer &WriteVarUInt(uintmax_t val);
	ByteBuffer &WriteString(std::string_view string);
	ByteBuffer &WriteBytes(std::span<const std::byte> bytes);

	std::span<const std::byte> Build() const;

	constexpr std::byte &Data() noexcept;
	constexpr size_t Size() const noexcept;
	constexpr bool Empty() const noexcept;

private:
	std::vector<std::byte> buffer;
};

template<ValidUIntType T>
inline constexpr T ByteBuffer::ReadUInt()
{
	constexpr size_t OUT_SIZE = sizeof(T);

	if (buffer.size() < OUT_SIZE)
		return 0;

	T out = 0;
	std::memcpy(&out, buffer.data(), OUT_SIZE);

	if constexpr (std::endian::native == std::endian::little)
		out = std::byteswap(out);

	auto iter = buffer.begin();
	std::advance(iter, OUT_SIZE);

	return out;
}

template<ValidUIntUnderlyingType T>
inline constexpr T ByteBuffer::ReadUInt()
{
	return static_cast<T>(ReadUInt<std::underlying_type_t<T>>(buffer));
}

template<ValidUIntType T>
inline ByteBuffer &ByteBuffer::WriteUInt(const T val)
{
	T out = val;

	if constexpr (std::same_as<T, uint8_t>)
		buffer.push_back(std::byte(out));

	else
	{
		if constexpr (std::endian::native == std::endian::little)
			out = std::byteswap(out);

		buffer.assign_range(std::as_bytes(std::span<const T>(&out, 1)));
	}

	return *this;
}

template<ValidUIntUnderlyingType T>
inline ByteBuffer &ByteBuffer::WriteUInt(const T val)
{
	return WriteUInt<std::underlying_type_t<T>>(static_cast<std::underlying_type_t<T>>(val));
}

#endif
