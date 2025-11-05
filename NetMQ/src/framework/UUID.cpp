#include <random>
#include <algorithm>
#include <functional>
#include <ranges>
#include <sstream>
#include <array>

#include "UUID.h"

namespace UUID
{
	namespace
	{
		template <typename T = std::mt19937_64, std::size_t N = T::state_size * sizeof(typename T::result_type)>
		T SeededRandomEngine()
		{
			std::random_device source;

			auto randomdata = std::views::iota(std::size_t(), (N - 1) / sizeof(source()) + 1)
				| std::views::transform([&](auto) { return source(); });

			std::seed_seq seeds(std::begin(randomdata), std::end(randomdata));
			return T(seeds);
		}

		std::uniform_int_distribution<> dis(0, 15);
		std::uniform_int_distribution<> dis2(8, 11);
	}
}

std::string UUID::GenerateV4()
{
	static std::mt19937_64 engine = SeededRandomEngine();

	std::stringstream ss;
	ss << std::hex;
	for (int i=0; i<8; i++)
		ss << dis(engine);

	ss << "-";
	for (int i=0; i<4; i++)
		ss << dis(engine);

	ss << "-4";
	for (int i=0; i<3; i++)
		ss << dis(engine);

	ss << "-" << dis2(engine);
	for (int i=0; i<3; i++)
		ss << dis(engine);

	ss << "-";
	for (int i=0; i<12; i++)
		ss << dis(engine);

	return ss.str();
}
