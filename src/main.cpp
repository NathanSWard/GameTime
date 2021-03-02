﻿#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "sdl/sdl.hpp"

#include <core/game.hpp>
#include <core/input/plugin.hpp>

int main(int, char**)
{
	auto const context = sdl::Context::create(SDL_INIT_VIDEO);
	if (!context) {
		spdlog::error("sdl::Context::create failed. Error: {}", context.error().msg);
		return EXIT_FAILURE;
	}

	auto window = sdl::Window::create("Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_RESIZABLE);
	if (!window) {
		spdlog::error("sdl::Window::create failed. Error: {}", window.error().msg);
		return EXIT_FAILURE;
	}

	Game g;
	g.add_plugin(InputPlugin{});

	return 0;
}
