#pragma once

#include "sdl/sdl.hpp"
#include "core/game.hpp"
#include "core/ecs/commands.hpp"
#include "core/ecs/resource.hpp"

struct Texture {
    sdl::Texture texture;
};

struct TextureAtlas {
    std::vector<SDL_Rect> textures;
};

struct Renderer {
    sdl::Renderer renderer;
};

struct Window {
    sdl::Window window;
};
