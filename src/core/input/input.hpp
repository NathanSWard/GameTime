#pragma once

#include <fmt/format.h>
#include <ranges>
#include <util/common.hpp>
#include <util/containers/hash.hpp>

struct ElementState
{
    enum
    {
        Pressed,
        Released,
    } state = Pressed;

    constexpr auto is_pressed() const noexcept -> bool
    {
        return state == Pressed;
    }
};

template <typename T>
class Input
{
    HashSet<T> m_pressed;
    HashSet<T> m_just_pressed;
    HashSet<T> m_just_released;

public:
    Input() noexcept = default;
    Input(Input&&) noexcept = default;
    Input& operator=(Input&&) noexcept = default;

    Input(Input const&) = delete;
    Input& operator=(Input const&) = delete;

    void press(T const& input)
    {
        auto [iter, inserted] = m_pressed.insert(input);
        UNUSED(iter);
        if (inserted) {
            m_just_pressed.insert(input);
        }
    }

    void release(T const& input)
    {
        m_pressed.erase(input);
        m_just_released.insert(input);
    }

    auto pressed(T const& input) const noexcept -> bool
    {
        return m_pressed.contains(input);
    }

    auto just_pressed(T const& input) const noexcept -> bool
    {
        return m_just_pressed.contains(input);
    }

    auto just_released(T const& input) const noexcept -> bool
    {
        return m_just_released.contains(input);
    }

    void reset(T const& input)
    {
        m_pressed.erase(input);
        m_just_pressed.erase(input);
        m_just_released.erase(input);
    }

    void update()
    {
        m_just_pressed.clear();
        m_just_released.clear();
    }

    auto get_pressed() const noexcept
    {
        return std::views::all(m_pressed);
    }

    auto get_just_pressed() const noexcept
    {
        return std::views::all(m_just_pressed);
    }

    auto get_just_released() const noexcept
    {
        return std::views::all(m_just_released);
    }
};
// Format Specifiers

template <>
struct fmt::formatter<ElementState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(ElementState const& mb, Ctx& ctx) {
        return format_to(ctx.out(), "ElementState::{}", mb.is_pressed() ? "Pressed" : "Released");
    }
};