#pragma once

#include <core/game/game.hpp>
#include <core/sprite/sprite.hpp>

struct SpritePlugin
{
    void build(GameBuilder& builder)
    {
        builder.prepare_components<Sprite>();
    }
};