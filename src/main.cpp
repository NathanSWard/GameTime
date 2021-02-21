#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "sdl/sdl.hpp"
#include "ecs/system.hpp"
#include <filesystem>

int main(int, char**)
{
	auto const sdl_context = sdl::Context::create(SDL_INIT_VIDEO);
	if (!sdl_context) {
		spdlog::error("sdl::Context::create failed. Error: {}", sdl_context.error().msg);
		return EXIT_FAILURE;
	}

	auto const img_context = sdl::img::Context::create(IMG_INIT_PNG | IMG_INIT_JPG);
	if (!img_context) {
		spdlog::error("sdl::img::Context::create failed. Error: {}", img_context.error().msg);
		return EXIT_FAILURE;
	}

	auto window = sdl::Window::create("Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_RESIZABLE);
	if (!window) {
		spdlog::error("sdl::Window::create failed. Error: {}", window.error().msg);
		return EXIT_FAILURE;
	}

	auto renderer = sdl::Renderer::create(window.value(), -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		spdlog::error("sdl::Renderer::create failed. Error: {}", renderer.error().msg);
		return EXIT_FAILURE;
	}

	printf("%s\n", std::filesystem::current_path().string().c_str());

	auto player_texture = sdl::Texture::create(renderer.value(), "../../../../src/assets/rainbow-heart.png");
	if (!player_texture) {
		spdlog::error("sdl::Texture::create failed. Error: {}", player_texture.error().msg);
		return EXIT_FAILURE;
	}

	entt::registry reg;
	auto const e = reg.create();
	reg.emplace<Position>(e, 100.f, 100.f);
	reg.emplace<Velocity>(e);
	reg.emplace<Speed>(e, 150.f);
	reg.emplace<Player>(e);
	reg.emplace<Sprite>(e, std::move(player_texture).value());
	reg.emplace<Size>(e, 100.f, 100.f);

	bool quit = false;

	using clock_t = std::chrono::high_resolution_clock;
	auto prev_time = clock_t::now();

	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT: {
				quit = true;
				break;
			}
			default: break;
			}
		}

		using dt_t = std::chrono::duration<float>;
		auto const now = clock_t::now();
		auto const dt = std::chrono::duration_cast<dt_t>(now - prev_time);
		prev_time = now;

		SDL_SetRenderDrawColor(renderer->raw(), 255, 255, 255, 1);
		SDL_RenderClear(renderer->raw());

		SDL_SetRenderDrawColor(renderer->raw(), 0, 0, 255, 1);

		PlayerControl(reg);
		Physics(dt.count(), reg);
		Draw(renderer->raw(), reg);

		SDL_RenderPresent(renderer->raw());
	}

	return 0;
}
