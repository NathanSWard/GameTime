#pragma once

#include <core/math/vec.hpp>
#include <core/window/event.hpp>
#include <sdl/sdl.hpp>
#include <string>
#include <string_view>
#include <tl/optional.hpp>
#include <util/common.hpp>

struct BorderSize
{
    int top, left, bottom, right = 0;
};

struct WindowSize
{
    int width, height = 0;
};

class Window
{
    sdl::Window m_window;

    Window(sdl::Window&& w) noexcept : m_window(MOV(w)) {}

public:
    constexpr Window(Window&&) noexcept = default;
    constexpr Window& operator=(Window&&) noexcept = default;

    auto create(WindowSettings const& settings) noexcept -> tl::expected<Window, sdl::Error>
    {
        auto sdl_win = sdl::Window::create(
            settings.title.c_str(),
            settings.x,
            settings.y,
            settings.width,
            settings.height,
            settings.descriptor.flags);

        if (sdl_win) {
            return Window(*MOV(sdl_win));
        }
        else {
            return tl::make_unexpected(MOV(sdl_win).error());
        }
    }

    auto descriptor() const noexcept -> WindowDescriptor
    {
        return WindowDescriptor{ .flags = SDL_GetWindowFlags(const_cast<SDL_Window*>(m_window.raw())) };
    }

    auto border_size() const noexcept -> BorderSize
    {
        int t, l, b, r = 0;
        SDL_GetWindowBordersSize(const_cast<SDL_Window*>(m_window.raw()), &t, &l, &b, &r);
        return BorderSize{ t, l, b, r };
    }

    auto brightness() const noexcept -> float
    {
        return SDL_GetWindowBrightness(const_cast<SDL_Window*>(m_window.raw()));
    }

    auto id() const noexcept -> WindowId
    {
        return WindowId{ .id = SDL_GetWindowID(const_cast<SDL_Window*>(m_window.raw())) };
    }
    
    auto maximum_size() const noexcept -> WindowSize
    {
        int w, h = -1;
        SDL_GetWindowMaximumSize(const_cast<SDL_Window*>(m_window.raw()), &w, &h);
        return WindowSize{ w, h };
    }

    auto minimum_size() const noexcept -> WindowSize
    {
        int w, h = -1;
        SDL_GetWindowMinimumSize(const_cast<SDL_Window*>(m_window.raw()), &w, &h);
        return WindowSize{ w, h };
    }

    auto opacity() const noexcept -> float
    {
        float o = 0.f;
        SDL_GetWindowOpacity(const_cast<SDL_Window*>(m_window.raw()), &o);
        return o;
    }
    
    auto position() const noexcept -> Vec2i
    {
        int x, y = -1;
        SDL_GetWindowSize(const_cast<SDL_Window*>(m_window.raw()), &x, &y);
        return Vec2i{ x, y };
    }
    
    auto size() const noexcept -> WindowSize
    {
        int w, h = -1;
        SDL_GetWindowSize(const_cast<SDL_Window*>(m_window.raw()), &w, &h);
        return WindowSize{ w, h };
    }

    auto title() const noexcept -> std::string_view
    {
        return std::string_view{ SDL_GetWindowTitle(const_cast<SDL_Window*>(m_window.raw())) };
    }
};