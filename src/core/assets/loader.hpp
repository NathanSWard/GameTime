#pragma once

#include <concepts>
#include <cstddef>
#include <span>
#include <string_view>
#include <string>
#include <tl/optional.hpp>

#include <util/common.hpp>
#include <util/void_ptr.hpp>

class LoadedAsset
{
    void_ptr m_data;
    type_id_t m_type_id;

    constexpr LoadedAsset(void_ptr&& data, type_id_t const tid)
        : m_data(MOV(data))
        , m_type_id(tid)
    {}

    friend class AssetServer;

public:
    constexpr LoadedAsset(LoadedAsset&&) noexcept = default;
    constexpr LoadedAsset& operator=(LoadedAsset&&) noexcept = default;

    template <typename T, typename... Args>
    [[nodiscard]] constexpr static auto create(Args&&... args) -> LoadedAsset
    {
        return LoadedAsset(void_ptr::create<T>(FWD(args)...), ::type_id<T>());
    }

    [[nodiscard]] constexpr auto type_id() const noexcept -> type_id_t
    {
        return m_type_id;
    }
};

struct AssetLoader
{
    virtual auto extensions() const noexcept -> std::span<std::string_view const> = 0;
    virtual auto load(std::string_view path, std::span<std::byte> bytes) const -> tl::optional<LoadedAsset> = 0;
};

template <typename T>
concept IsAssetLoader = std::is_base_of_v<AssetLoader, T>;