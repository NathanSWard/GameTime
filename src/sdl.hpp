#pragma once

#include <SDL2/SDL.h>
#include <string_view>
#include <tl/expected.hpp>

namespace sdl {

    struct Error {
        std::string_view msg;

        static auto current() noexcept -> Error {
            return Error{
                .msg = SDL_GetError(),
            };
        }
    };

    class Context {
        bool m_ok = true;

        constexpr Context() noexcept = default;
    
    public:
        Context(Context const&) = delete;
        constexpr Context(Context&& other) noexcept : m_ok(std::exchange(other.m_ok, false)) {}

        static auto create(std::uint32_t const flags) noexcept -> tl::expected<Context, Error> {
            if (SDL_Init(flags) < 0) {
                return tl::make_unexpected(Error::current());
            }
            return Context{};
        }

        ~Context() noexcept { if (m_ok) SDL_Quit(); }
    };

    class Window {
        SDL_Window* m_window = nullptr;

        constexpr Window(SDL_Window* const window) noexcept
            : m_window(window)
        {}

    public:
        Window(Window const&) = delete;
        constexpr Window(Window&& other) noexcept : m_window(std::exchange(other.m_window, nullptr)) {}

        static auto create(char const* const title, int const x, int const y, int const w, int const h, std::uint32_t const flags) -> tl::expected<Window, Error> {
            auto const window = SDL_CreateWindow(title, x, y, w, h, flags);
            if (window == nullptr) {
                return tl::make_unexpected(Error::current());
            }
            return Window(window);
        }

        constexpr auto raw() noexcept -> SDL_Window* { return m_window; }
        constexpr auto raw() const noexcept -> SDL_Window const* { return m_window; }

        ~Window() noexcept { if (m_window) SDL_DestroyWindow(m_window); }
    };

}