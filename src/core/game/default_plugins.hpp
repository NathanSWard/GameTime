#pragma once

#include <assets/plugin.hpp>
#include <game/game.hpp>
#include <input/plugin.hpp>

struct DefaultPlugins
{
    void build(GameBuilder& builder)
    {
        builder
            .add_plugin(InputPlugin{})
            .add_plugin(AssetPlugin{});
    }
};