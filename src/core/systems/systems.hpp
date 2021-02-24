#pragma once

#include "core/components/components.hpp"
#include <entt/entt.hpp>
#include <SDL2/SDL.h>
#include "core/ecs/system.hpp"
#include "core/resources/resources.hpp"

void Physics(Resource<Time> time, Query<With<Position, Velocity const>> query) {
	auto const dt = time->delta.count();
	query.each([dt](Position& p, Velocity const& v) {
		p.x += v.dx * dt;
		p.y += v.dy * dt;
		});
}

void Draw(Resource<Renderer> renderer, Resource<Texture> texture, Resource<TextureAtlas> atlas, Query<With<Position const, Size const, AnimationState>> query) {
	SDL_SetRenderDrawColor(renderer->renderer.raw(), 255, 255, 255, 1);
	SDL_RenderClear(renderer->renderer.raw());

	query.each([renderer, texture, atlas](Position const& p, Size const& s, AnimationState& a_state) mutable {
		auto rect = SDL_Rect{
			.x = (int)p.x,
			.y = (int)p.y,
			.w = (int)s.w,
			.h = (int)s.h,
		};

		auto const& currentClip = atlas->textures[a_state.sprite_index];



		SDL_RenderCopy(renderer->renderer.raw(), texture->texture.raw(), &currentClip, &rect);

		++a_state.sprite_index;

		if (a_state.sprite_index >= a_state.total_sprites) {
			a_state.sprite_index = 0;
		}

		});
	SDL_RenderPresent(renderer->renderer.raw());
}

void PlayerControl(Query<With<Player const, Velocity, Speed const>> query) {

	SDL_Event e;
	while (SDL_PollEvent(&e)) {}

	query.each([](Velocity& v, Speed const& s) {
		auto const* const key_state = SDL_GetKeyboardState(nullptr);

		if (key_state[SDL_SCANCODE_W] || key_state[SDL_SCANCODE_S]) {
			v.dy = key_state[SDL_SCANCODE_W] ? -s.speed : s.speed;
		}
		else {
			v.dy = 0.f;
		}
		if (key_state[SDL_SCANCODE_A] || key_state[SDL_SCANCODE_D]) {
			v.dx = key_state[SDL_SCANCODE_A] ? -s.speed : s.speed;
		}
		else {
			v.dx = 0.f;
		}
		});
}