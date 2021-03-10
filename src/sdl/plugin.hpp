#pragma once

#include <core/assets/asset_server.hpp>
#include <debug/debug.hpp>
#include <sdl/sdl.hpp>
#include <sdl/sdl_event.hpp>
#include <sdl/sdl_img_loader.hpp>
#include <sdl/sdl_runner.hpp>

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
            PANIC("SDL2 failed. Error: {}", sdl_ctx.error().msg);
        }

        auto img_ctx = sdl::img::Context::create(IMG_INIT_PNG);
        if (!img_ctx) {
            PANIC("SDL2_image failed. Error: {}", img_ctx.error().msg);
        }

        builder.set_resource<SDLContext>(*MOV(sdl_ctx), *MOV(img_ctx));

        auto server = builder.resources().get_resource<AssetServer>();
        if (!server) {
            PANIC("SDLPlugin requires the `AssetServer` resource");
        }
        (*server)->add_asset_loader<SDL_IMG_Loader>();

        builder.set_runner(sdl_runner);
    }
};