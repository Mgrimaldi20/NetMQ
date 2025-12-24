#include <stdexcept>

#include "ByteBuffer.h"

std::byte &ByteBuffer::operator[](size_t index) noexcept
{
	return buffer[index];
}

ByteBuffer::ByteBuffer(std::span<std::byte> buf)
	: buffer(buf.begin(), buf.end())
{
}

ByteBuffer::ByteBuffer(std::vector<std::byte> buf)
	: buffer(std::move(buf))
{
}

constexpr uintmax_t ByteBuffer::ReadVarUInt()
{
	uintmax_t multiplier = 1;
	uintmax_t value = 0;
	std::byte encoded = std::byte(0);

	do
	{
		encoded = ReadUInt<std::byte>();
		value += (std::to_integer<uintmax_t>(encoded) & 127) * multiplier;

		constexpr uintmax_t MAX_MULTIPLIER = 128 * 128 * 128;
		if (multiplier > MAX_MULTIPLIER)
			throw std::runtime_error("Malformed Variable Byte Integer");

		multiplier *= 128;
	} while (std::to_integer<uintmax_t>(encoded) & 128);

	return value;
}

constexpr std::string ByteBuffer::ReadString(size_t length)
{
	if (buffer.size() < length)
		return "";

	std::string out;
	std::transform(buffer.begin(), buffer.end(), std::back_inserter(out), [](std::byte b) { return static_cast<char>(b); });

	auto iter = buffer.begin();
	std::advance(iter, length);

	return out;
}

constexpr std::span<const std::byte> ByteBuffer::ReadBytes(size_t length)
{
	if (buffer.size() < length)
		return std::span<const std::byte>();

	std::span<const std::byte> out(buffer.data(), length);

	auto iter = buffer.begin();
	std::advance(iter, length);

	return out;
}

ByteBuffer &ByteBuffer::WriteVarUInt(uintmax_t val)
{
	do
	{
		std::byte encoded = std::byte(val % 128);
		val /= 128;

		if (val > 0)
			encoded |= std::byte(128);

		buffer.push_back(encoded);
	} while (val > 0);

	return *this;
}

ByteBuffer &ByteBuffer::WriteString(std::string_view string)
{
	for (char c : string)
		buffer.push_back(std::byte(c));

	return *this;
}

ByteBuffer &ByteBuffer::WriteBytes(std::span<const std::byte> bytes)
{
	buffer.assign_range(bytes);
	return *this;
}

std::span<const std::byte> ByteBuffer::Build() const
{
	return std::span<const std::byte>(buffer);
}

constexpr std::byte &ByteBuffer::Data() noexcept
{
	return *buffer.data();
}

constexpr size_t ByteBuffer::Size() const noexcept
{
	return buffer.size();
}

constexpr bool ByteBuffer::Empty() const noexcept
{
	return buffer.empty();
}
