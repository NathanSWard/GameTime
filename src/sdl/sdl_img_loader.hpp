#pragma once

#include <array>
#include <core/assets/loader.hpp>
#include <core/render/texture.hpp>

struct SDL_IMG_Loader final : public AssetLoader
{
    static constexpr auto exts = std::array<std::string_view, 1>{ "png" };
    
    auto extensions() const noexcept -> std::span<std::string_view const> final
    {
        return std::span<std::string_view const>{ exts.data(), exts.size() };
    }
    
    auto load(std::string_view, std::span<std::byte> const bytes) const -> tl::optional<LoadedAsset>
    {
        auto* const surface = IMG_Load_RW(SDL_RWFromMem(bytes.data(), static_cast<int>(bytes.size())), 1);
        if (surface == nullptr) {
            return {};
        }
        return LoadedAsset::create<Texture>(surface);
    }
};