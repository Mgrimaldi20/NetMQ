#include "Cmd.h"

size_t CmdUtil::ReadU32BigEndian(const std::span<const std::byte> &buffer, const size_t offset, uint32_t &out) noexcept
{
	if ((buffer.size() - out) < 4)
		return 0;

	out = (std::to_integer<uint32_t>(buffer[offset + 0]) << 24)
		| (std::to_integer<uint32_t>(buffer[offset + 1]) << 16)
		| (std::to_integer<uint32_t>(buffer[offset + 2]) << 8)
		| (std::to_integer<uint32_t>(buffer[offset + 3]));

	return sizeof(uint32_t);
}
