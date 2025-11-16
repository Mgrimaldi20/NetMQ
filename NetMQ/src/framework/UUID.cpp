#include <random>
#include <algorithm>
#include <functional>
#include <ranges>
#include <sstream>
#include <iomanip>
#include <array>

#include "UUID.h"

namespace NetMQ::UUID
{
	namespace
	{
		inline std::uint64_t GenerateRandomUInt64()
		{
			std::random_device rd;
			std::uniform_int_distribution<uint64_t> dis;
			return dis(rd);
		}
	}
}

std::string NetMQ::UUID::GenerateV4()
{
	std::uint64_t rand = GenerateRandomUInt64();
	std::uint64_t rand1 = GenerateRandomUInt64();

	std::stringstream ss;
	ss << std::hex << std::setfill('0');

	ss << std::setw(8) << static_cast<std::uint32_t>(rand >> 32) << "-";
	ss << std::setw(4) << static_cast<std::uint16_t>(rand >> 16) << "-";
	ss << std::setw(4) << ((static_cast<std::uint16_t>(rand) & 0x0FFF) | 0x4000) << "-";
	ss << std::setw(2) << static_cast<int>(((static_cast<uint8_t>(rand1 >> 56) & 0x3F) | 0x80)) << "-";
	ss << std::setw(2) << static_cast<int>(static_cast<uint8_t>(rand1 >> 48)) << "-";
	ss << std::setw(12) << (rand1 & 0x0000FFFFFFFFFFFFULL);

	return ss.str();
}
