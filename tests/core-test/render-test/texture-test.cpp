#include <ut.hpp>
#include <core/render/texture.hpp>

using namespace boost::ut;

void texture_test()
{
    "[Texture/Assets<Texture>]"_test = [] {
        auto win = sdl::Window::create("text", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 40, 40, 0);
        expect((win.has_value()) >> fatal);
        auto ren = sdl::Renderer::create(*win, -1, 0);
        expect((ren.has_value()) >> fatal);

        auto rctx = RenderContext(*MOV(ren));
        auto rctx_resource = make_resource<RenderContext>(rctx);

        auto surface = SDL_CreateRGBSurface(0, 10, 10, 32, 0, 0, 0, 0);
        expect((surface != nullptr) >> fatal);

        auto texture = SDL_CreateTextureFromSurface(rctx.raw(), surface);
        expect((texture != nullptr) >> fatal);

        auto assets = Assets<Texture>();

        // add a new surface & texture
        auto shandle = assets.add_asset(surface);
        auto thandle = assets.add_asset(texture);

        expect(assets.size() == 1);
        expect(assets.unready_texture_size() == 1);
        expect(!assets.contains_asset(shandle.id()));
        expect(assets.contains_asset(thandle.id()));

        sdl_surface_to_texture_system(assets, rctx_resource);

        expect(assets.size() == 2);
        expect(assets.unready_texture_size() == 0);
        expect(assets.contains_asset(shandle.id()));
        expect(assets.contains_asset(thandle.id()));

        // change the now texture to a surface & texture
        auto surface2 = SDL_CreateRGBSurface(0, 10, 10, 32, 0, 0, 0, 0);
        expect((surface2 != nullptr) >> fatal);

        auto texture2 = SDL_CreateTextureFromSurface(rctx.raw(), surface2);
        expect((texture2 != nullptr) >> fatal);

        assets.set_asset(shandle, surface2);
        expect(!assets.contains_asset(shandle.id()));

        assets.set_asset(thandle, texture2);
        expect(assets.contains_asset(thandle.id()));

        expect(assets.size() == 1);
        expect(assets.unready_texture_size() == 1);

        sdl_surface_to_texture_system(assets, rctx_resource);

        expect(assets.size() == 2);
        expect(assets.unready_texture_size() == 0);
        expect(assets.contains_asset(shandle.id()));
        expect(assets.contains_asset(thandle.id()));
    };
}