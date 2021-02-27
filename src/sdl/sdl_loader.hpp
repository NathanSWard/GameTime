#pragma once

#include <array>
#include <core/assets/loader.hpp>
#include "sdl.hpp"

#include <iostream>

struct SDLLoader final : public AssetLoader
{
    static constexpr auto exts = std::array<std::string_view, 1>{ "png" };
    
    auto extensions() const noexcept -> std::span<std::string_view const> final
    {
        return std::span<std::string_view const>{ exts.data(), exts.size() };
    }
    
    auto load(std::string_view, std::span<std::byte const> const bytes) const -> tl::optional<LoadedAsset>
    {
        auto* const surface = IMG_Load_RW(SDL_RWFromMem(const_cast<std::byte*>(bytes.data()), static_cast<int>(bytes.size())), 1);
        if (surface == nullptr) {
            std::cout << "Error: " << IMG_GetError() << '\n';
            return {};
        }
        return LoadedAsset::create<sdl::Surface>(sdl::Surface::from_raw(surface));
    }
};