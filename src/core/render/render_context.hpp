#pragma once

#include <core/window/window.hpp>
#include <sdl/sdl.hpp>
#include <util/common.hpp>

struct RenderContextSettings
{
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
    static auto create(RenderContextSettings const& settings, Window& window) -> tl::expected<RenderContext, sdl::Error>
    {
        auto renderer = sdl::Renderer::create(window.m_window, -1, settings.flags);
        if (!renderer) {
            return tl::make_unexpected(renderer.error());
        }

        return RenderContext(*MOV(renderer));
    }

    constexpr RenderContext(RenderContext&&) noexcept = default;
    constexpr RenderContext& operator=(RenderContext&&) noexcept = default;

    [[nodiscard]] constexpr auto raw() const noexcept { return m_renderer.raw(); }
    [[nodiscard]] constexpr auto raw() noexcept { return m_renderer.raw(); }

    // TODO: Add more getters/setters
};