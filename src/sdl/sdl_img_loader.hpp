#pragma once

#include <array>
#include <core/assets/loader.hpp>
#include <sdl/sdl.hpp>

struct SDL_IMG_Loader final : public AssetLoader
{
    static constexpr auto exts = std::array<std::string_view, 1>{ "png" };
    
    auto extensions() const noexcept -> std::span<std::string_view const> final
    {
        return std::span<std::string_view const>{ exts.data(), exts.size() };
    }
    
    auto load(std::string_view, std::span<std::byte const> const bytes) const -> tl::optional<LoadedAsset>
    {
        auto* const surface = IMG_Load_RW(SDL_RWFromMem(const_cast<std::byte*>(bytes.data()), static_cast<int>(bytes.size())), 1);
        return LoadedAsset::create<sdl::Surface>(sdl::Surface::from_raw(surface));
    }
};