#pragma once

#include "util/common.hpp"
#include <entt/core/type_info.hpp>
#include <tl/optional.hpp>
#include <type_traits>
#include <unordered_map>

#include "void_ptr.hpp"

class TypeMap {
    std::unordered_map<type_id_t, void_ptr> m_map;

public:

    [[nodiscard]] auto size() const noexcept -> std::size_t
    {
        return m_map.size();
    }

    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return m_map.empty();
    }

    void clear() noexcept
    {
        m_map.clear();
    }

    template <typename T, typename... Args>
    auto insert(Args&&... args) -> T&
    {
        auto const [iter, ok] = m_map.insert_or_assign(type_id<T>(), void_ptr::create<T>(FWD(args)...));
        UNUSED(ok);
        return *static_cast<T*>(iter->second.data());
    }

    template <typename T>
    [[nodiscard]] auto contains() const -> bool
    {
        return m_map.contains(entt::type_id<T>().hash());
    }

    template <typename T>
    auto remove() -> std::unique_ptr<T>
    {
        if (auto const iter = m_map.find(type_id<T>()); iter == m_map.end()) {
            return nullptr;
        }
        else {
            auto const ptr = static_cast<T*>(iter->second.take());
            m_map.erase(iter);
            return std::unique_ptr<T>(ptr);
        }
    }

    template <typename T>
    [[nodiscard]] auto get() -> tl::optional<T&>
    {
        if (auto const iter = m_map.find(type_id<T>()); iter == m_map.end()) {
            return {};
        } 
        else {
            T* ptr = static_cast<T*>(iter->second.data());
            return tl::make_optional<T&>(*ptr);
        }
    }

    template <typename T>
    [[nodiscard]] auto get() const -> tl::optional<T const&>
    {
        if (auto const iter = m_map.find(type_id<T>()); iter == m_map.end()) {
            return {};
        }
        else {
            T const* ptr = static_cast<T const*>(iter->second.data());
            return tl::make_optional<T const&>(*ptr);
        }
    }

    template <typename T>
    [[nodiscard]] auto cget() const ->tl::optional<T const&>
    {
        return get();
    }
};