#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "sdl/sdl.hpp"
#include "core/plugins/sdl.hpp"
#include "core/plugins/time.hpp"
#include "core/systems/systems.hpp"
#include "core/ecs/system.hpp"
#include "core/components/components.hpp"
#include "core/resources/resources.hpp"

void MakeEntities(Commands cmds) {
	auto window = sdl::Window::create("Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_RESIZABLE);
	if (!window) {
		spdlog::error("sdl::Window::create failed. Error: {}", window.error().msg);
		std::exit(1);
	}

	auto renderer = sdl::Renderer::create(window.value(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		spdlog::error("sdl::Renderer::create failed. Error: {}", renderer.error().msg);
		std::exit(1);
	}

	auto player_texture = sdl::Texture::create(renderer.value(), "../../../../src/assets/earth.png");
	if (!player_texture) {
		spdlog::error("sdl::Texture::create failed. Error: {}", player_texture.error().msg);
		std::exit(1);
	}

	cmds.add_resource<Window>(MOV(window).value());
	cmds.add_resource<Renderer>(MOV(renderer).value());
	cmds.add_resource<Texture>(MOV(player_texture).value());

	std::vector<SDL_Rect> rects;
	for (int i = 0; i < 1024; i += 1024 / 16) {
		for (int j = 0; j < 1024; j += 1024 / 16) {
			rects.push_back(SDL_Rect{
				.x = j,
				.y = i,
				.w = 1024 / 16,
				.h = 1024 / 16
				});
		}
	}
	cmds.add_resource<TextureAtlas>(MOV(rects));

	auto const e = cmds.spawn();
	cmds.add_component<Position>(e, 40.f, 40.f);
	cmds.add_component<Velocity>(e);
	cmds.add_component<Speed>(e, 150.f);
	cmds.add_component<Player>(e);
	cmds.add_component<Sprite>(e);
	cmds.add_component<Size>(e, 100.f, 100.f);
	cmds.add_component<TextureAtlas>(e, std::move(rects));
	cmds.add_component<AnimationState>(e, AnimationState{
			.sprite_index = 0,
			.total_sprites = 16 * 16
		});
}

int main(int, char**)
{
	Game game;

	game.add_plugin(TimePlugin{});
	auto const sdl_result = game.add_plugin(SDLPlugin{});
	if (!sdl_result) {
		auto const& error = sdl_result.error();
		spdlog::error("SDLPlugin failed.");
		return EXIT_FAILURE;
	}

	game.add_system(Physics);
	game.add_system(Draw);
	game.add_system(PlayerControl);
	
	game.add_startup_system(MakeEntities);

	game.run();

	return 0;
}
