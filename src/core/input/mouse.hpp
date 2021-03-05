#pragma once

#include <core/ecs/resource.hpp>
#include <core/game/events.hpp>
#include <core/input/input.hpp>
#include <core/math/vec.hpp>
#include <fmt/format.h>
#include <functional>

class MouseButton
{
public:
    enum Type : std::uint16_t {
        Left,
        Right,
        Middle,
        Other,
    };

    struct OtherButton { std::uint16_t button = 0;  };

private:
    Type m_type = Type::Other;
    OtherButton m_other = OtherButton{ 0 };

public:
    constexpr explicit MouseButton(Type const type) noexcept
        : m_type(type)
    {}

    constexpr explicit MouseButton(OtherButton const other) noexcept
        : m_type(Type::Other)
        , m_other(other)
    {}

    constexpr MouseButton(MouseButton&&) noexcept = default;
    constexpr MouseButton(MouseButton const&) noexcept = default;
    constexpr MouseButton& operator=(MouseButton&&) noexcept = default;
    constexpr MouseButton& operator=(MouseButton const&) noexcept = default;

    constexpr auto type() const noexcept -> Type { return m_type; }
    constexpr auto other() const noexcept -> OtherButton { return m_other; }

    constexpr auto operator==(MouseButton const& lhs) const noexcept -> bool
    {
        return m_type == lhs.type() && m_other.button == lhs.other().button;
    }

    constexpr auto operator!=(MouseButton const& lhs) const noexcept -> bool
    {
        return !(*this == lhs);
    }
};

template <>
struct std::hash<MouseButton>
{
    auto operator()(MouseButton const& mbutton) const noexcept -> std::size_t
    {
        auto const type = static_cast<std::uint16_t>(mbutton.type());
        auto const other = mbutton.other().button;
  
        std::uint32_t const hash = std::uint32_t{ type } << 16 | other;
        return static_cast<std::size_t>(hash);
    }
};

struct MouseButtonInput
{
    MouseButton button;
    ElementState state;
    std::uint8_t clicks = 1;
};

struct MouseMotion
{
    Vec2 delta;
};

struct MouseWheel
{
    float x = 0.f;
    float y = 0.f;
};

void mouse_button_input_system(
    Resource<Input<MouseButton>> mouse_button_input,
    EventReader<MouseButtonInput> mouse_button_input_events)
{
    mouse_button_input->update();
    auto events = mouse_button_input_events.iter();
    for (auto const& e : events) {
        switch (e.state.state) {
            case ElementState::Pressed: 
                mouse_button_input->press(e.button); 
                break;
            case ElementState::Released: 
                mouse_button_input->release(e.button); 
                break;
            default: // unreachable
                break;
        }
    }
}

// Format Specifiers

template <>
struct fmt::formatter<MouseButton> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(MouseButton const& mb, Ctx& ctx) {
        switch (mb.type()) {
            case MouseButton::Left: return format_to(ctx.out(), "MouseButton::Left");
            case MouseButton::Right: return format_to(ctx.out(), "MouseButton::Right");
            case MouseButton::Middle: return format_to(ctx.out(), "MouseButton::Middle");
            case MouseButton::Other: return format_to(ctx.out(), "MouseButton::Other({})", mb.other().button);
        }
    }
};

template <>
struct fmt::formatter<MouseButtonInput> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(MouseButtonInput const& mbi, Ctx& ctx) {
        return format_to(
            ctx.out(),
            "MouseButtonInput(button: {}, state: {}, clicks: {})",
            mbi.button,
            mbi.state,
            mbi.clicks);
    }
};

template <>
struct fmt::formatter<MouseMotion> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(MouseMotion const& mm, Ctx& ctx) {
        return format_to(
            ctx.out(),
            "MouseMotion(delta: ({}, {}))",
            mm.delta.x(),
            mm.delta.y());
    }
};

template <>
struct fmt::formatter<MouseWheel> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(MouseWheel const& mw, Ctx& ctx) {
        return format_to(
            ctx.out(),
            "MouseWheel(x: {}, y: {})", 
            mw.x, 
            mw.y);
    }
};