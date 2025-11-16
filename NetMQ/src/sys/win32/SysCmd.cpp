#include <cstddef>
#include <mutex>
#include <memory>
#include <functional>
#include <unordered_map>
#include <span>
#include <vector>

#include "io/IOContext.h"

struct ByteHash
{
	using is_transparent = void;

	std::size_t operator()(const std::vector<std::byte> &v) const
	{
		return HashBytes(std::span<const std::byte>(v));
	}

	std::size_t operator()(std::span<std::byte> s) const
	{
		return HashBytes(s);
	}

private:
	std::size_t HashBytes(std::span<const std::byte> bytes) const
	{
		std::size_t seed = bytes.size();
		for (std::byte b : bytes)
			seed ^= static_cast<std::size_t>(b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}
};

struct ByteEquality
{
	using is_transparent = void;

	bool operator()(const std::vector<std::byte> &lhs, const std::vector<std::byte> &rhs) const
	{
		return lhs == rhs;
	}

	bool operator()(const std::vector<std::byte> &lhs, std::span<const std::byte> rhs) const
	{
		return std::span<const std::byte>(lhs).size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	bool operator()(std::span<const std::byte> lhs, const std::vector<std::byte> &rhs) const
	{
		return operator()(rhs, lhs);
	}
};

static std::unordered_map<std::vector<std::byte>, std::vector<std::shared_ptr<IOContext>>, ByteHash, ByteEquality> subscriptions;
static std::mutex subsmtx;
