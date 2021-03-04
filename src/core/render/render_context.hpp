#pragma once

#include <util/common.hpp>
#include <sdl/sdl.hpp>

class RenderContext
{
    sdl::Renderer m_renderer;

public:
    constexpr RenderContext(sdl::Renderer&& renderer) noexcept
        : m_renderer(MOV(renderer))
    {}

    constexpr RenderContext(RenderContext&&) noexcept = default;
    constexpr RenderContext& operator=(RenderContext&&) noexcept = default;

    constexpr auto raw() const noexcept { return m_renderer.raw(); }
    constexpr auto raw() noexcept { return m_renderer.raw(); }
};