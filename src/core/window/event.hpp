#pragma once

#include <core/math/vec.hpp>
#include <functional>
#include <filesystem>
#include <SDL2/SDL.h>

struct WindowId { std::uint32_t id = 0; };

constexpr auto operator==(WindowId const& lhs, WindowId const& rhs) noexcept
{
    return lhs.id == rhs.id;
}

constexpr auto operator!=(WindowId const& lhs, WindowId const& rhs) noexcept
{
    return !(lhs == rhs);
}

template <>
struct std::hash<WindowId>
{
    auto operator()(WindowId const& id) const noexcept -> std::size_t
    {
        return id.id;
    }
};

struct WindowDescriptor
{
    std::uint32_t flags = SDL_WINDOW_RESIZABLE;

    auto shown() const noexcept -> bool { return (flags & SDL_WINDOW_SHOWN); }
    WindowDescriptor& set_shown(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_SHOWN : flags & (~SDL_WINDOW_SHOWN);
        return *this;
    }

    auto hidden() const noexcept -> bool { return (flags & SDL_WINDOW_HIDDEN); }
    WindowDescriptor& set_hidden(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_HIDDEN : flags & (~SDL_WINDOW_HIDDEN);
        return *this;
    }

    auto borderless() const noexcept -> bool { return (flags & SDL_WINDOW_BORDERLESS); }
    WindowDescriptor& set_borderless(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_BORDERLESS : flags & (~SDL_WINDOW_BORDERLESS);
        return *this;
    }

    auto resizable() const noexcept -> bool { return (flags & SDL_WINDOW_RESIZABLE); }
    WindowDescriptor& set_resizable(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_RESIZABLE : flags & (~SDL_WINDOW_RESIZABLE);
        return *this;
    }

    auto minimized() const noexcept -> bool { return (flags & SDL_WINDOW_MINIMIZED); }
    WindowDescriptor& set_minimized(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_MINIMIZED : flags & (~SDL_WINDOW_MINIMIZED);
        return *this;
    }

    auto maximized() const noexcept -> bool { return (flags & SDL_WINDOW_MAXIMIZED); }
    WindowDescriptor& set_maximized(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_MAXIMIZED : flags & (~SDL_WINDOW_MAXIMIZED);
        return *this;
    }

    auto input_grabbed() const noexcept -> bool { return (flags & SDL_WINDOW_INPUT_GRABBED); }
    WindowDescriptor& set_input_grabbed(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_INPUT_GRABBED : flags & (~SDL_WINDOW_INPUT_GRABBED);
        return *this;
    }

    auto input_focus() const noexcept -> bool { return (flags & SDL_WINDOW_INPUT_FOCUS); }
    WindowDescriptor& set_input_focus(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_INPUT_FOCUS : flags & (~SDL_WINDOW_INPUT_FOCUS);
        return *this;
    }

    auto mouse_focus() const noexcept -> bool { return (flags & SDL_WINDOW_MOUSE_FOCUS); }
    WindowDescriptor& set_mouse_focus(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_MOUSE_FOCUS : flags & (~SDL_WINDOW_MOUSE_FOCUS);
        return *this;
    }

    auto mouse_captured() const noexcept -> bool { return (flags & SDL_WINDOW_MOUSE_CAPTURE); }
    WindowDescriptor& set_mouse_captured(bool const b = true) noexcept
    {
        flags = b ? flags | SDL_WINDOW_MOUSE_CAPTURE : flags & (~SDL_WINDOW_MOUSE_CAPTURE);
        return *this;
    }
};

struct WindowSettings
{
    static constexpr int position_undefined = SDL_WINDOWPOS_UNDEFINED;
    static constexpr int position_centered = SDL_WINDOWPOS_CENTERED;

    std::string title = "Title";
    int x, y = position_undefined;
    int width = 1280;
    int height = 720;

    WindowDescriptor descriptor;
};

// events

struct WindowResized
{
    WindowId id;
    int width, height = 0;
};

struct InitializeWindow
{
    WindowSettings settings;
};

struct CloseWindow
{
    WindowId id;
};

struct WindowCreated
{
    WindowId id;
};

struct WindowCloseRequest
{
    WindowId id;
};

struct CursorMoved
{
    WindowId id;
    Vec2 position;
};

struct CursorEntered
{
    WindowId id;
};

struct CursorLeft
{
    WindowId id;
};

struct WindowFocused
{
    WindowId id;
    bool focused = true;
};

struct FileDragAndDrop
{
    WindowId id;
    std::filesystem::path path;
};

struct WindowMoved
{
    WindowId id;
    Vec2i position;
};