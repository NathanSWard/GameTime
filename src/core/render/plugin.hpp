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
            .add_system_to_stage<RenderStage>(render_draw_system)
            .add_event<InitializeRenderContext>();

        auto& resources = builder.resources();
        auto render_ctx_settings = resources
            .get_resource<RenderContextSettings>()
            .map([](auto const& r) { return *r; })
            .value_or(RenderContextSettings{});

        auto create_window_event = resources.get_resource<Events<InitializeRenderContext>>().value();
        create_window_event->send(InitializeRenderContext{
            .settings = render_ctx_settings
            });
    }
};