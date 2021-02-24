#pragma once

#include "core/game.hpp"
#include "sdl/sdl.hpp"
#include <tl/expected.hpp>
#include <tl/optional.hpp>

struct SDLContext
{
	sdl::Context sdl_context;
	sdl::img::Context img_context;
};

struct SDLError
{
	tl::optional<sdl::Error> sdl_error;
	tl::optional<sdl::img::Error> img_error;
};

struct SDLPlugin
{
    auto build(Game& game) -> tl::expected<void, SDLError>
    {
        auto sdl_context = sdl::Context::create(SDL_INIT_VIDEO);
		if (!sdl_context) {
			return tl::make_unexpected(SDLError{
				.sdl_error = sdl_context.error(),
				.img_error = tl::nullopt,
				});
		}

		auto img_context = sdl::img::Context::create(IMG_INIT_PNG | IMG_INIT_JPG);
		if (!img_context) {
			return tl::make_unexpected(SDLError{
				.sdl_error =  tl::nullopt, 
				.img_error = img_context.error(),
				});
		}

		game.add_resource<SDLContext>(MOV(sdl_context).value(), MOV(img_context).value());
		return {};
    }
};