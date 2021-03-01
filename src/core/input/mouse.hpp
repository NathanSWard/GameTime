#pragma once

#include <core/ecs/resource.hpp>
#include <core/events/events.hpp>
#include <core/input/input.hpp>
#include <core/math/vec.hpp>
#include <functional>

class MouseButton
{
public:
    enum class Type : std::uint16_t {
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
};

struct MouseMotion
{
    Vec2 delta;
};

enum class MouseScrollUnit : bool
{
    Line,
    Pixel,
};

struct MouseWheel
{
    MouseScrollUnit unit = MouseScrollUnit::Line;
    float x = 0.f;
    float y = 0.f;
};

void mouse_button_input_system(
    Resource<Input<MouseButton>> mouse_button_input,
    EventReader<MouseButtonInput> mouse_button_input_events)
{
    mouse_button_input->update();
    for (auto const& e : mouse_button_input_events.iter()) {
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