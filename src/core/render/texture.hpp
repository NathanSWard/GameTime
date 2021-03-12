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

    auto raw_texture() noexcept -> SDL_Texture*
    {
        DEBUG_ASSERT(m_is_texture, "Texture has not be updated from a surface to texture yet.");
        return m_texture;
    }

    ~Texture() noexcept { destroy(); }
};

template <>
class Assets<Texture> : public AssetsBase<Texture>
{
    using vec_t = std::vector<std::pair<HandleId, Texture>>;
    vec_t m_surfaces;

public:
    auto unready_texture_size() const noexcept -> std::size_t { return m_surfaces.size(); }

    Assets(Sender<RefChange> sender)
        : AssetsBase<Texture>(MOV(sender))
    {}

    Assets(Assets&&) noexcept = default;
    Assets& operator=(Assets&&) noexcept = default;

    template <typename... Args>
    auto add_asset(Args&&... args) -> Handle<Texture>
    {
        auto const id = HandleId::random<Texture>();
        auto texture = Texture(FWD(args)...);

        m_events.push_back(AssetEvent<Texture>::created(Handle<Texture>::weak(id)));
        if (texture.m_is_texture) {
            this->m_assets.insert_or_assign(id, MOV(texture));
        }
        else {
            m_surfaces.emplace_back(std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(MOV(texture)));
        }

        return get_handle(id);
    }

    template <typename... Args>
    auto set_asset(HandleId const id, Args&&... args)
    {
        auto texture = Texture(FWD(args)...);

        if (auto const tfound = m_assets.find(id); tfound != m_assets.end()) {
            if (texture.m_is_texture) {
                tfound->second = MOV(texture);
            }
            else {
                m_assets.erase(tfound);
                m_surfaces.emplace_back(std::piecewise_construct, 
                    std::forward_as_tuple(id), 
                    std::forward_as_tuple(MOV(texture)));
            }
            m_events.push_back(AssetEvent<Texture>::modified(Handle<Texture>::weak(id)));
        }
        else if (auto const sfound = std::ranges::find(m_surfaces, id, [](auto const& data) -> HandleId const& { return data.first; })
            ; sfound != m_surfaces.end()) {
            if (texture.m_is_texture) {
                m_surfaces.erase(sfound);
                m_assets.insert_or_assign(id, MOV(texture));
            }
            else {
                sfound->second = MOV(texture);
            }
            m_events.push_back(AssetEvent<Texture>::modified(Handle<Texture>::weak(id)));
        }
        else {
            if (texture.m_is_texture) {
                m_assets.insert_or_assign(id, MOV(texture));
            }
            else {
                m_surfaces.emplace_back(std::piecewise_construct,
                    std::forward_as_tuple(id),
                    std::forward_as_tuple(MOV(texture)));
            }
            m_events.push_back(AssetEvent<Texture>::created(Handle<Texture>::weak(id)));
        }
    }

    void update(RenderContext& rctx)
    {
        if (m_surfaces.empty()) {
            return;
        }

        for (auto& [id, asset] : m_surfaces) {
            DEBUG_ASSERT(!asset.m_is_texture, "`Texture` is already an SDL_Texture.");

            auto* const sdl_surface = asset.m_surface;
            auto* const sdl_texture = SDL_CreateTextureFromSurface(rctx.raw(), sdl_surface);

            asset.m_texture = sdl_texture;
            asset.m_is_texture = true;

            SDL_FreeSurface(sdl_surface);

            m_assets.insert_or_assign(MOV(id), MOV(asset));
        }

        vec_t().swap(m_surfaces);
    }
};

void sdl_update_texture_assets_system(Resource<Assets<Texture>> assets, Resource<RenderContext> rctx)
{
    assets->update(*rctx);
}