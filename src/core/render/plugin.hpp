#pragma once

#include <core/game/game.hpp>
#include <core/window/window.hpp>
#include "color.hpp"
#include "draw.hpp"
#include "render_context.hpp"
#include "system.hpp"
#include "texture.hpp"

struct RenderStage {};

struct RenderPlugin
{
    void build(GameBuilder& builder)
    {
        builder
            .prepare_components<Visible, Transparent, Color>()
            .add_asset<Texture>()
            .add_stage_after<RenderStage, CoreStages::PostUpdate>()
            .add_system(sdl_update_texture_assets_system)
            .add_system_to_stage<RenderStage>(render_draw_system);

        auto& resources = builder.resources();
        auto const rctx_settings = resources
            .get_resource<RenderContextSettings>()
            .map([](auto const& r) { return *r; })
            .value_or(RenderContextSettings{});

        auto window = resources.get_resource<Window>();
        if (!window) {
            panic("RenderPlugin requires `Window` resource to exist.");
            return;
        }

        auto rctx = RenderContext::create(rctx_settings, **window);
        if (!rctx) {
            panic("RenderContext failed to initialize. Error: {}", panic_args(rctx.error().msg));
            return;
        }

        builder.set_resource<RenderContext>(*MOV(rctx));
    }
};