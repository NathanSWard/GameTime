#pragma once

#include <core/assets/plugin.hpp>
#include <core/game/game.hpp>
#include <core/input/plugin.hpp>
#include <core/render/plugin.hpp>
#include <core/sprite/plugin.hpp>
#include <core/window/plugin.hpp>
#include <sdl/plugin.hpp>

struct DefaultPlugins
{
    void build(GameBuilder& builder)
    {
        builder
            .add_plugin(InputPlugin{})
            .add_plugin(AssetPlugin{})
            .add_plugin(WindowPlugin{})
            .add_plugin(SDLPlugin{})
            .add_plugin(RenderPlugin{})
            .add_plugin(SpritePlugin{});
    }
};