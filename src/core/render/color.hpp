#pragma once

#include <tuple>

struct Color 
{
    std::uint8_t r, g, b, a = 0;

    [[nodiscard]] constexpr auto as_float() const noexcept 
    {
        return std::tuple{
                float(r / 255.0f),
                float(g / 255.0f),
                float(b / 255.0f),
                float(a / 255.0f)
        };
    }

    inline static constexpr Color black() noexcept { return { 0, 0, 0, 255 }; }; 
    inline static constexpr Color white() noexcept { return { 255, 255, 255, 255 }; };
    inline static constexpr Color red() noexcept { return { 255, 0, 0, 255 }; };
    inline static constexpr Color orange() noexcept { return { 255, 128, 0, 255 }; };
    inline static constexpr Color yellow() noexcept { return { 255, 255, 0, 255 }; };
    inline static constexpr Color green() noexcept { return { 0, 255, 0, 255 }; };
    inline static constexpr Color blue() noexcept { return { 0, 0, 255, 255 }; };
    inline static constexpr Color purple() noexcept { return { 127, 0, 255, 255 }; };

    [[nodiscard]] inline static constexpr auto lerp(Color const& a, Color const& b, float const t) noexcept -> Color
    {
        return {
                std::uint8_t((float)a.r + ((b.r - a.r) * t)),
                std::uint8_t((float)a.g + ((b.g - a.g) * t)),
                std::uint8_t((float)a.b + ((b.b - a.b) * t)),
                std::uint8_t((float)a.a + ((b.a - a.a) * t))
        };
    }
};