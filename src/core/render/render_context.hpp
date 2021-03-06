#pragma once

#include <core/window/window.hpp>
#include <sdl/sdl.hpp>
#include <util/common.hpp>

struct RenderContextSettings
{
    WindowId window_id;
    std::uint32_t flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
};

struct InitializeRenderContext
{
    RenderContextSettings settings;
};

class RenderContext
{
    sdl::Renderer m_renderer;

    RenderContext(sdl::Renderer&& renderer) noexcept : m_renderer(MOV(renderer)) {}

public:
    static auto create(RenderContextSettings const& settings, Windows& windows) -> tl::optional<RenderContext>
    {
        auto window = windows.get(settings.window_id);
        if (!window) {
            panic("Renderer unable to find window with id: {}", panic_args(settings.window_id));
            return {};
        }

        auto renderer = sdl::Renderer::create(window->m_window, -1, settings.flags);
        if (!renderer) {
            panic("Renderer creation failed. Error: {}", panic_args(renderer.error().msg));
            return {};
        }

        return RenderContext(*MOV(renderer));
    }

    constexpr RenderContext(RenderContext&&) noexcept = default;
    constexpr RenderContext& operator=(RenderContext&&) noexcept = default;

    [[nodiscard]] constexpr auto raw() const noexcept { return m_renderer.raw(); }
    [[nodiscard]] constexpr auto raw() noexcept { return m_renderer.raw(); }

    // TODO: Add more getters/setters
};