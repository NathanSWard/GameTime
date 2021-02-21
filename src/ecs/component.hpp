#pragma once
#include "../sdl/sdl.hpp"

struct Player {};

// Drawing
struct Sprite {
	sdl::Texture texture;
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