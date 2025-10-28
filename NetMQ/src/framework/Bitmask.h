#ifndef _NETMQ_BITMASK_H_
#define _NETMQ_BITMASK_H_

#include <type_traits>
#include <utility>

namespace Bitmask
{
	template <typename T>
	struct EnableBitmaskOperators : public std::false_type {};

	template <typename T>
	constexpr T operator~(T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		return static_cast<T>(~std::to_underlying(rhs));
	}

	template <typename T>
	constexpr T operator|(T lhs, T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		return static_cast<T>(std::to_underlying(lhs) | std::to_underlying(rhs));
	}

	template <typename T>
	constexpr T operator&(T lhs, T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		return static_cast<T>(std::to_underlying(lhs) & std::to_underlying(rhs));
	}

	template <typename T>
	constexpr T operator^(T lhs, T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		return static_cast<T>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
	}

	template <typename T>
	constexpr T operator<<(T lhs, int rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		return static_cast<T>(std::to_underlying(lhs) << rhs);
	}

	template <typename T>
	constexpr T operator>>(T lhs, int rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		return static_cast<T>(std::to_underlying(lhs) >> rhs);
	}

	template <typename T>
	constexpr T &operator|=(T &lhs, T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		lhs = static_cast<T>(std::to_underlying(lhs) | std::to_underlying(rhs));
		return lhs;
	}

	template <typename T>
	constexpr T &operator&=(T &lhs, T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		lhs = static_cast<T>(std::to_underlying(lhs) & std::to_underlying(rhs));
		return lhs;
	}

	template <typename T>
	constexpr T &operator^=(T &lhs, T rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		lhs = static_cast<T>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
		return lhs;
	}

	template <typename T>
	constexpr T &operator<<=(T &lhs, int rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		lhs = static_cast<T>(std::to_underlying(lhs) << rhs);
		return lhs;
	}

	template <typename T>
	constexpr T &operator>>=(T &lhs, int rhs) noexcept requires EnableBitmaskOperators<T>::value
	{
		lhs = static_cast<T>(std::to_underlying(lhs) >> rhs);
		return lhs;
	}

	template <typename T>
	constexpr bool HasFlag(T value, T flag) noexcept requires EnableBitmaskOperators<T>::value
	{
		return (value & flag) == flag;
	}

	template <typename T, unsigned int B>
	consteval T Bit() noexcept requires EnableBitmaskOperators<T>::value
	{
		static_assert(B < (sizeof(std::underlying_type_t<T>) * 8), "Bit index out of range for type T");
		return static_cast<T>(static_cast<std::underlying_type_t<T>>(1) << B);
	}
}

#endif
