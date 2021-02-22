#pragma once

#include "component.hpp"
#include <entt/entt.hpp>
#include <SDL2/SDL.h>

void Physics(float dt, entt::registry& reg) {
	auto view = reg.view<Position, Velocity const>();
	view.each([dt](Position& p, Velocity const& v) {
		p.x += v.dx * dt;
		p.y += v.dy * dt;
		});
}

void Draw(SDL_Renderer* renderer, entt::registry& reg) {
	auto view = reg.view<Position const, Size const, Sprite, TextureAtlas const, AnimationState>();
	view.each([renderer](Position const& p, Size const& s, Sprite& sprite, TextureAtlas const& atlas, AnimationState& a_state) {
		auto rect = SDL_Rect{
			.x = (int)p.x,
			.y = (int)p.y,
			.w = (int)s.w,
			.h = (int)s.h,
		};

		auto const& currentClip = atlas.textures[a_state.sprite_index];

		SDL_RenderCopy(renderer, sprite.texture.raw(), &currentClip, &rect);

		++a_state.sprite_index;

		if (a_state.sprite_index >= a_state.total_sprites) {
			a_state.sprite_index = 0;
		}

		fmt::print("{}\n", a_state.sprite_index);
		
		});

}

void PlayerControl(entt::registry& reg) {
	auto view = reg.view<Player const, Velocity, Speed const>();
	view.each([](Velocity& v, Speed const& s) {
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