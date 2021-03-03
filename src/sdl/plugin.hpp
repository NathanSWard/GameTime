#pragma once

#include <core/assets/asset_server.hpp>
#include <debug/debug.hpp>
#include <sdl/sdl.hpp>
#include <sdl/sdl_event.hpp>
#include <sdl/sdl_img_loader.hpp>

struct SDLContext 
{
    sdl::Context sdl_ctx;
    sdl::img::Context img_ctx;
};

struct SDLPlugin
{
    // TODO: Texture/Surface types
    void build(GameBuilder& builder)
    {
        
        auto sdl_ctx = sdl::Context::create(SDL_INIT_EVERYTHING);
        if (!sdl_ctx) {
            panic("SDL2 failed. Error: {}", panic_args(sdl_ctx.error().msg));
        }
        
        auto img_ctx = sdl::img::Context::create(IMG_INIT_PNG);
        if (!img_ctx) {
            panic("SDL2_image failed. Error: {}", panic_args(img_ctx.error().msg));
        }

        builder
            .set_resource<SDLContext>(*MOV(sdl_ctx), *MOV(img_ctx))
            .add_system_to_stage<CoreStages::PreEvents>(sdl_event_system);

        auto server = builder.resources().get_resource<AssetServer>();
        if (!server) {
            panic("SDLPlugin requires the `AssetServer` resource");
        }
        (*server)->add_asset_loader<SDL_IMG_Loader>();
    }
};