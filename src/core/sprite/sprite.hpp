#pragma once

#include <core/ecs/world.hpp>
#include <core/math/vec.hpp>
#include <core/math/transform.hpp>
#include <core/render/color.hpp>
#include <core/render/draw.hpp>
#include <core/render/texture.hpp>
#include <core/assets/handle.hpp>
#include <tl/optional.hpp>

struct Sprite
{
    Vec2 size;
    bool flip_x = false;
    bool flip_y = false;
};

// TODO: Possibly wrap Handle<Texture> & Color in their own structs
struct SpriteBundle
{
    Sprite sprite;
    Transform transform;
    Handle<Texture> texture;
    tl::optional<Color> color;
    bool is_visible = true;
    bool is_transparent = false;

    void build(World& world) &&
    {
        auto const e = world.create();
        world.emplace<Sprite>(e, MOV(sprite));
        world.emplace<Handle<Texture>>(e, MOV(texture));
        world.emplace<Transform>(e, MOV(transform));
        
        if (color) {
            world.emplace<Color>(e, *color);
        }
        if (is_visible) {
            world.emplace<Visible>(e);
        }
        if (is_transparent) {
            world.emplace<Transparent>(e);
        }
    }
};