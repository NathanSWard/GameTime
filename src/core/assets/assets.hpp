#pragma once

#include <tl/optional.hpp>
#include <unordered_map>

#include "handle.hpp"
#include <util/common.hpp>

template <typename T>
class Assets 
{
    std::unordered_map<HandleId, T> m_assets;

public:
    Assets() noexcept = default;

    Assets(Assets&&) noexcept = default;
    Assets& operator=(Assets&&) noexcept = default;

    Assets(Assets const&) = delete;
    Assets& operator=(Assets const&) = delete;

    template <typename... Args>
    auto add_asset(Args&&... args) -> Handle<T>
    {
        auto const id = HandleId::random<T>();
        m_assets.insert_or_assign(id, T(FWD(args)...)); // will always suceed
        return Handle<T>{ id };
    }

    template <typename... Args>
    auto set_asset(Handle<T> const& handle, Args&&... args) -> Handle<T>
    {
        auto const id = handle.id();
        m_assets.insert_or_assign(id, T(FWD(args)...));
        return Handle<T>{ id };
    }

    [[nodiscard]] auto contains_asset(Handle<T> const& handle) const noexcept -> bool
    {
        return m_assets.contains(handle.id());
    }

    [[nodiscard]] auto get_asset(Handle<T> const& handle) -> tl::optional<T&>
    {
        if (auto const iter = m_assets.find(handle.id()); iter != m_assets.end()) {
            return tl::make_optional<T&>(iter->second);
        }
        return {};
    }

    [[nodiscard]] auto get_asset(Handle<T> const& handle) const -> tl::optional<T const&>
    {
        if (auto const iter = m_assets.find(handle.id()); iter != m_assets.end()) {
            return tl::make_optional<T const&>(iter->second);
        }
        return {};
    }

    [[nodiscard]] auto cget_asset(Handle<T> const& handle) const -> tl::optional<T const&>
    {
        return get_asset(handle);
    }

    auto remove_asset(Handle<T> const& handle) -> tl::optional<T>
    {
        auto const id = handle.id();
        if (auto const iter = m_assets.find(id); iter != m_assets.end()) {
            auto value = MOV(iter->second);
            m_assets.erase(iter);
            return tl::make_optional<T>(MOV(value));
        }
        return {};
    }

    auto get_handle(HandleId const id) -> Handle<T>
    {
        return Handle<T>{ id };
    }

    [[nodiscard]] auto begin() { return m_assets.begin(); }
    [[nodiscard]] auto end() { return m_assets.begin(); }
    [[nodiscard]] auto begin() const { return m_assets.begin(); }
    [[nodiscard]] auto end() const { return m_assets.begin(); }

    void clear() { m_assets.clear(); }
    void reserve(std::size_t const size) { m_assets.reserve(size); }
    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_assets.size(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return m_assets.empty(); }
};

