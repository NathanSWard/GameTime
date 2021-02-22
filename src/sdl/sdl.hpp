#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

    namespace img {
        struct Error {
            std::string_view msg;

            static auto current() noexcept -> Error {
                return Error{
                    .msg = IMG_GetError(),
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
                if (IMG_Init(flags) < 0) {
                    return tl::make_unexpected(Error::current());
                }
                return Context{};
            }

            ~Context() noexcept { if (m_ok) IMG_Quit(); }
        };
    }

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

    class Renderer {
        SDL_Renderer* m_renderer = nullptr;

        constexpr Renderer(SDL_Renderer* const renderer) noexcept
            : m_renderer(renderer)
        {}

    public:
        Renderer(Renderer const&) = delete;
        constexpr Renderer(Renderer&& other) noexcept : m_renderer(std::exchange(other.m_renderer, nullptr)) {}

        static auto create(Window& window, int const index, std::uint32_t const flags) -> tl::expected<Renderer, Error> {
            auto const renderer = SDL_CreateRenderer(window.raw(), index, flags);
            if (renderer == nullptr) {
                return tl::make_unexpected(Error::current());
            }
            return Renderer(renderer);
        }

        constexpr auto raw() noexcept -> SDL_Renderer* { return m_renderer; }
        constexpr auto raw() const noexcept -> SDL_Renderer const* { return m_renderer; }

        ~Renderer() noexcept { if (m_renderer) SDL_DestroyRenderer(m_renderer); }
    };
}