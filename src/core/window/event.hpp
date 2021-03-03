#pragma once

#include <core/math/vec.hpp>
#include <SDL2/SDL.h>

struct WindowId { std::uint32_t id = 0; };

struct CursorMoved
{
    WindowId id;
    Vec2 position;
};