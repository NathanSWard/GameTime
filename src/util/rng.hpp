#pragma once

#include <bit>
#include <concepts>
#include <limits>
#include <random>

namespace util {

    template <std::integral I, typename Engine = std::mt19937>
    I uniform_rand(I const min, I const max)
    {
        using param_type = typename std::uniform_int_distribution<I>::param_type;

        thread_local Engine eng{ std::random_device{}() };
        thread_local std::uniform_int_distribution<I> dist;

        return dist(eng, param_type{ min, max });
    }

    template <std::floating_point F, typename Engine = std::mt19937>
    F uniform_rand(F const min, F const max)
    {
        using param_type = typename std::uniform_real_distribution<F>::param_type;

        thread_local Engine eng{ std::random_device{}() };
        thread_local std::uniform_real_distribution<F> dist;

        return dist(eng, param_type{ min, max });
    }


    template <typename T>
    T uniform_rand()
    {
        return uniform_rand(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    namespace {
        std::uint64_t fast_rand_impl() noexcept
        {
            thread_local std::uint64_t shuffle_table[4] = { 0x41, 0x29837592, 0, 0 };

            std::uint64_t s1 = shuffle_table[0];
            std::uint64_t const s0 = shuffle_table[1];
            std::uint64_t const result = s0 + s1;

            shuffle_table[0] = s0;
            s1 ^= s1 << 23;
            shuffle_table[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);

            return result;
        }
    } // namespace

    template <std::integral I>
    I fast_rand() noexcept
    {
        return static_cast<I>(fast_rand_impl());
    }

    template <std::floating_point F>
    F fast_rand() noexcept
    {
        if constexpr (sizeof(F) == sizeof(std::uint32_t)) {
            std::uint32_t const x = static_cast<std::uint32_t>(fast_rand_impl());
            return std::bit_cast<F>(x);
        }
        else if (sizeof(F) == sizeof(std::uint64_t)) {
            std::uint64_t const x = fast_rand_impl();
            return std::bit_cast<F>(x);
        }
        else {
            return static_cast<F>(fast_rand_impl());
        }
    }

} // namespace util