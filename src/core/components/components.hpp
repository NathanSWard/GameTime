#pragma once

#include "sdl/sdl.hpp"

struct Player {};

// Drawing
struct Sprite {
};

struct AnimationState {
	int sprite_index = 0;
	int total_sprites = 0;
};

struct Size {
	float w, h = 0.f;
};

// Physics
struct Speed {
	float speed = 0.f;
};

struct Position {
	float x, y = 0.f;
};

struct Velocity {
	float dx, dy = 0.f;
};