#pragma once

#include <core/assets/assets.hpp>
#include <core/ecs/resource.hpp>
#include <core/render/render_context.hpp>
#include <debug/debug.hpp>
#include <ranges>
#include <SDL2/SDL.h>

struct Texture
{
    union {
        SDL_Surface* m_surface = nullptr;
        SDL_Texture* m_texture;
    };
    bool m_is_texture = false;

    template <typename>
    friend class Assets;

    void destroy()
    {
        if (m_is_texture) {
            if (m_texture) {
                SDL_DestroyTexture(m_texture);
            }
        }
        else {
            if (m_surface) {
                SDL_FreeSurface(m_surface);
            }
        }
    }

public:

    constexpr Texture(SDL_Surface* const surface) noexcept
        : m_surface(surface)
        , m_is_texture(false)
    {}

    constexpr Texture(SDL_Texture* const texture) noexcept
        : m_texture(texture)
        , m_is_texture(true)
    {}

    constexpr Texture(Texture&& other) noexcept
        : m_is_texture(other.m_is_texture)
    {
        if (m_is_texture) {
            m_texture = std::exchange(other.m_texture, nullptr);
        } 
        else {
            m_surface = std::exchange(other.m_surface, nullptr);
        }
    }

    Texture& operator=(Texture&& other) noexcept
    {
        destroy();
        m_is_texture = other.m_is_texture;
        if (m_is_texture) {
            m_texture = std::exchange(other.m_texture, nullptr);
        }
        else {
            m_surface = std::exchange(other.m_surface, nullptr);
        }
        return *this;
    }

    ~Texture() noexcept { destroy(); }
};

template <>
class Assets<Texture> : AssetsBase<Texture>
{
    std::unordered_map<HandleId, Texture> m_surfaces;
    friend void sdl_surface_to_texture_system(Assets<Texture>&, Resource<RenderContext>);

public:
    template <typename... Args>
    auto add_asset(Args&&... args) -> Handle<Texture>
    {
        auto const id = HandleId::random<Texture>();
        auto texture = Texture(FWD(args)...);

        if (texture.m_is_texture) {
            this->m_assets.insert_or_assign(id, MOV(texture));
        }
        else {
            m_surfaces.insert_or_assign(id, MOV(texture));
        }

        return Handle<Texture>{ id };
    }

    template <typename... Args>
    auto set_asset(Handle<Texture> const& handle, Args&&... args) -> Handle<Texture>
    {
        auto const id = handle.id();
        auto texture = Texture(FWD(args)...);

        if (auto const sfound = m_surfaces.find(id); sfound != m_surfaces.end()) {
            if (texture.m_is_texture) {
                m_surfaces.erase(sfound);
                m_assets.insert_or_assign(id, MOV(texture));
            } 
            else {
                *sfound = MOV(texture);
            }
        }
        else if (auto const tfound = m_assets.find(id); tfound != m_assets.end()) {
            if (texture.m_is_texture) {
                *tfound = MOV(texture);
            }
            else {
                m_surfaces.insert_or_assign(id, MOV(texture));
            }
        }
        else {
            if (texture.m_is_texture) {
                m_assets.insert_or_assign(id, MOV(texture));
            }
            else {
                m_surfaces.insert_or_assign(id, MOV(texture));
            }
        }

        return Handle<Texture>{ id };
    }
};

void sdl_surface_to_texture_system(Assets<Texture>& assets, Resource<RenderContext> rcontext)
{
    if (assets.m_surfaces.empty()) {
        return;
    }

    for (auto& [id, asset] : assets.m_surfaces) {
        DEBUG_ASSERT(!asset.m_is_texture, "`Texture` is already an SDL_Texture.");

        auto* const sdl_surface = asset.m_surface;
        auto* const sdl_texture = SDL_CreateTextureFromSurface(rcontext->raw(), sdl_surface);
        
        asset.m_texture = sdl_texture;
        asset.m_is_texture = true;

        SDL_FreeSurface(sdl_surface);

        assets.m_assets.insert_or_assign(MOV(id), MOV(asset));
    }

    assets.m_surfaces.clear();
}