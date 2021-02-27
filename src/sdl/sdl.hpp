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

    class Surface {
        SDL_Surface* m_surface = nullptr;

        constexpr Surface(SDL_Surface* const surface) noexcept
            : m_surface(surface)
        {}

    public:
        Surface(Surface const&) = delete;
        Surface(Surface&& other) noexcept : m_surface(std::exchange(other.m_surface, nullptr)) {}

        Surface& operator=(Surface&& other) noexcept {
            if (m_surface) {
                SDL_FreeSurface(m_surface);
            }
            m_surface = std::exchange(other.m_surface, nullptr);
            return *this;
        }

        static auto from_raw(SDL_Surface* const surface) -> Surface
        {
            return Surface(surface);
        }

        constexpr auto raw() noexcept -> SDL_Surface* { return m_surface; }
        constexpr auto raw() const noexcept -> SDL_Surface const* { return m_surface; }

        ~Surface() noexcept { if (m_surface) SDL_FreeSurface(m_surface); }
    };

    class Texture {
        SDL_Texture* m_texture = nullptr;

        constexpr Texture(SDL_Texture* const texture) noexcept
            : m_texture(texture)
        {}

    public:
        Texture(Texture const&) = delete;
        constexpr Texture(Texture&& other) noexcept : m_texture(std::exchange(other.m_texture, nullptr)) {}

        Texture& operator=(Texture&& other) noexcept {
            if (m_texture) {
                SDL_DestroyTexture(m_texture);
            }
            m_texture = std::exchange(other.m_texture, nullptr);
            return *this;
        }

        static auto from_surface(Renderer& rend, Surface& surface) -> tl::expected<Texture, Error>
        {
            auto const texture = SDL_CreateTextureFromSurface(rend.raw(), surface.raw());
            if (texture == nullptr) {
                return tl::make_unexpected(Error::current());
            }
            return Texture(texture);
        }

        static auto create(Renderer& rend, char const* const path) -> tl::expected<Texture, Error> {
            auto const surface = IMG_Load(path);
            if (surface == nullptr) {
                return tl::make_unexpected(Error::current());
            }

            auto const texture = SDL_CreateTextureFromSurface(rend.raw(), surface);
            SDL_FreeSurface(surface);
            if (texture == nullptr) {
                return tl::make_unexpected(Error::current());
            }

            return Texture(texture);
        }

        constexpr auto raw() noexcept -> SDL_Texture* { return m_texture; }
        constexpr auto raw() const noexcept -> SDL_Texture const* { return m_texture; }

        ~Texture() noexcept { if (m_texture) SDL_DestroyTexture(m_texture); }
    };
}