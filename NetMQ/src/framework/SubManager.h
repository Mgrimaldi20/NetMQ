#ifndef _NETMQ_SUBMANAGER_H_
#define _NETMQ_SUBMANAGER_H_

#include <cstddef>
#include <mutex>
#include <memory>
#include <vector>
#include <span>
#include <unordered_map>

#include "sys/win32/io/IOContext.h"

struct SubManager
{
	class ByteHash
	{
	public:
		using is_transparent = void;

		std::size_t operator()(const std::vector<std::byte> &v) const;
		std::size_t operator()(std::span<std::byte> s) const;

	private:
		std::size_t HashBytes(std::span<const std::byte> bytes) const;
	};

	class ByteEquality
	{
	public:
		using is_transparent = void;

		bool operator()(const std::vector<std::byte> &lhs, const std::vector<std::byte> &rhs) const;
		bool operator()(const std::vector<std::byte> &lhs, std::span<const std::byte> rhs) const;
		bool operator()(std::span<const std::byte> lhs, const std::vector<std::byte> &rhs) const;
	};

	std::unordered_map<std::vector<std::byte>, std::vector<std::shared_ptr<IOContext>>, ByteHash, ByteEquality> subscriptions;
	std::mutex subsmtx;
};

#endif
