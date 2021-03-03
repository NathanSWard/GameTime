#include "sdl.hpp"

struct SDLContext 
{
    sdl::Context sdl_ctx;
    sdl::img::Context img_ctx;
};

struct SDLPlugin
{
    void build(GameBuilder& builder)
    {
        // TODO
    }
};