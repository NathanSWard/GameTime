#pragma once

#include <core/ecs/query.hpp>
#include <core/ecs/resource.hpp>
#include <core/sprite/sprite.hpp>
#include "texture.hpp"
#include "render_context.hpp"

namespace {

    void render_draw_system_impl(
        RenderContext& ctx,
        Texture& texture,
        Sprite const& sprite,
        Transform const& tform)
    {
        auto dstrect = SDL_FRect{
            .x = tform.translation.x(),
            .y = tform.translation.y(),
            .w = sprite.size.x() * tform.scale.x(),
            .h = sprite.size.y() * tform.scale.y(),
        };

        SDL_RenderCopyExF(
            ctx.raw(),
            texture.raw_texture(),
            nullptr,
            &dstrect,
            tform.rotation,
            nullptr,
            static_cast<SDL_RendererFlip>(sprite.flip_state));
    }

} // namespace

void render_draw_system(
    Resource<RenderContext> ctx,
    Resource<Assets<Texture>> textures,
    Query<With<Sprite const, Handle<Texture> const, Transform const, Visible const>, Without<Color>> sprites_to_draw,
    Query<With<Sprite const, Handle<Texture> const, Transform const, Color const, Visible const>> colored_sprites_to_draw)
{
    SDL_SetRenderDrawColor(ctx->raw(), 255, 255, 255, 255);
    SDL_RenderClear(ctx->raw());

    sprites_to_draw.each([&](Sprite const& sprite, Handle<Texture> const& thandle, Transform const& tform) {
        auto texture = textures->get_asset(thandle);
        if (!texture) {
            return;
        }
        render_draw_system_impl(*ctx, *texture, sprite, tform);
        });



    colored_sprites_to_draw.each([&](Sprite const& sprite, Handle<Texture> const& thandle, Transform const& tform, Color const& color) {
        auto texture = textures->get_asset(thandle);
        if (!texture) {
            return;
        }

        // get the previous color/alpha mod
        std::uint8_t r, g, b, a = 0;

        SDL_GetTextureAlphaMod(texture->raw_texture(), &a);
        SDL_GetTextureColorMod(texture->raw_texture(), &r, &g, &b);

        bool const same_alpha = a == color.a;
        bool const same_color = (r == color.r && g == color.g && b == color.b);

        // set the new color/alpha mod
        if (!same_alpha) {
            SDL_SetTextureAlphaMod(texture->raw_texture(), color.a);
        }
        if (!same_color) {
            SDL_SetTextureColorMod(texture->raw_texture(), color.r, color.g, color.b);
        }

        // draw the texture
        render_draw_system_impl(*ctx, *texture, sprite, tform);

        // reset to the old color/alpha mod
        if (!same_alpha) {
            SDL_SetTextureAlphaMod(texture->raw_texture(), a);
        }
        if (!same_color) {
            SDL_SetTextureColorMod(texture->raw_texture(), r, g, b);
        }
        });

    SDL_RenderPresent(ctx->raw());
}