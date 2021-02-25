#pragma once

#include <compare>
#include <cstdint>
#include <functional>
#include <util/rng.hpp>

class Uuid {
    std::uint64_t m_a;
    std::uint64_t m_b;

    friend class std::hash<Uuid>;

public:
    constexpr Uuid(std::uint64_t const a, std::uint64_t const b) noexcept 
        : m_a(a)
        , m_b(b)
    {}

    constexpr Uuid(Uuid&&) noexcept = default;
    constexpr Uuid(Uuid const&) noexcept = default;
    constexpr Uuid& operator=(Uuid&&) noexcept = default;
    constexpr Uuid& operator=(Uuid const&) noexcept = default;

    static auto create() noexcept -> Uuid
    {
        auto const a = util::uniform_rand<std::uint64_t>();
        auto const b = util::uniform_rand<std::uint64_t>();
        return Uuid{ a, b };
    }

    constexpr auto operator<=>(Uuid const& rhs) const noexcept = default;
};

template <>
struct std::hash<Uuid>
{
    auto operator()(Uuid const& uuid) const noexcept -> std::size_t 
    {
        auto a = uuid.m_a;
        auto const b = uuid.m_b;
        a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
        return static_cast<std::size_t>(a);
    }
};