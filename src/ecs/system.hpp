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
	auto view = reg.view<Position const, Size const, Sprite>();
	view.each([renderer](Position const& p, Size const& s, Sprite& sprite) {
		auto rect = SDL_FRect{
			.x = p.x,
			.y = p.y,
			.w = s.w,
			.h = s.h,
		};

		SDL_RenderCopyF(renderer, sprite.texture.raw(), nullptr, &rect);
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