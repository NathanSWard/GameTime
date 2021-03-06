#pragma once

#include <core/math/vec.hpp>

struct Transform
{
    Vec2 translation = Vec2{ 0.f, 0.f };
    Vec2 scale = Vec2{ 1.f, 1.f };
    float rotation = 0.f; // in degrees
};