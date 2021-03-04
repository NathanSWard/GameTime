#pragma once

#include <sdl/sdl.hpp>

class RenderContext
{
    sdl::Renderer m_renderer;

public:

    constexpr auto raw() const noexcept { return m_renderer.raw(); }
    constexpr auto raw() noexcept { return m_renderer.raw(); }
};