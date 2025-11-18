#include "SubManager.h"

std::size_t SubManager::ByteHash::operator()(const std::vector<std::byte> &v) const
{
	return HashBytes(std::span<const std::byte>(v));
}

std::size_t SubManager::ByteHash::operator()(std::span<std::byte> s) const
{
	return HashBytes(s);
}

std::size_t SubManager::ByteHash::HashBytes(std::span<const std::byte> bytes) const
{
	std::size_t seed = bytes.size();
	for (std::byte b : bytes)
		seed ^= static_cast<std::size_t>(b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

	return seed;
}

bool SubManager::ByteEquality::operator()(const std::vector<std::byte> &lhs, const std::vector<std::byte> &rhs) const
{
	return lhs == rhs;
}

bool SubManager::ByteEquality::operator()(const std::vector<std::byte> &lhs, std::span<const std::byte> rhs) const
{
	return std::span<const std::byte>(lhs).size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

bool SubManager::ByteEquality::operator()(std::span<const std::byte> lhs, const std::vector<std::byte> &rhs) const
{
	return operator()(rhs, lhs);
}
