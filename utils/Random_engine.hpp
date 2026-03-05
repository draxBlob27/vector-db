#ifndef RANDOM_ENGINE_HPP
#define RANDOM_ENGINE_HPP
#include <chrono>
#include <random>

namespace Random {
    inline std::mt19937 generate()
	{
		std::random_device rd{};

		// Create seed_seq with clock and 7 random numbers from std::random_device
		std::seed_seq ss{
			static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
				rd(), rd(), rd(), rd(), rd(), rd(), rd() };

		return std::mt19937{ ss };
	}

    // Here's our global std::mt19937 object.
	// The inline keyword means we only have one global instance for our whole program.
    inline std::mt19937 mt{ generate() };

    template <typename T>
	T get(T min, T max)
	{
		return std::normal_distribution<T>{min, max}(mt);
	}

    template <typename R, typename S, typename T>
	R get(S min, T max)
	{
		return get<R>(static_cast<R>(min), static_cast<R>(max));
	}
}
#endif //RANDOM_ENGINE_HPP