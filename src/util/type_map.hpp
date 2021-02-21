#pragma once

#include "type_id.hpp"
#include <unordered_map>
#include <tl/optional.hpp>

#include <entt/core/type_info.hpp>

class TypeMap {
    std::unordered_map<typeid_t, void*> m_map;

public:

    std::size_t size() const noexcept
    {
        return m_map.size();
    }

    auto empty() const noexcept -> bool
    {
        return m_map.empty();
    }

    template <typename T, typename... Args>
    T& insert(Args&&... args) 
    {
        T* ptr = new T(std::forward<Args>(args)...);
        auto const [iter, ok] = m_map.insert_or_assign(type_id<T>(), static_cast<void*>(ptr));
        static_cast<void>(ok);
        return *static_cast<T*>(iter->second);
    }

    template <typename T>
    auto remove() -> std::unique_ptr<T>
    {
        if (auto const iter = m_map.find(type_id<T>()); iter == m_map.end()) {
            return nullptr;
        }
        else {
            auto const ptr = static_cast<T*>(iter->second);
            m_map.erase(iter);
            return std::unique_ptr<T>(ptr);
        }
    }

    template <typename T>
    auto get() -> tl::optional<T&>
    {
        if (auto const iter = m_map.find(type_id<T>()); iter == m_map.end()) {
            return {};
        } 
        else {
            T* ptr = static_cast<T*>(iter->second);
            return tl::make_optional<T&>(*ptr);
        }
    }

    template <typename T>
    auto get() const -> tl::optional<T const&>
    {
        if (auto const iter = m_map.find(type_id<T>()); iter == m_map.end()) {
            return {};
        }
        else {
            T const* ptr = static_cast<T const*>(*iter);
            return tl::make_optional<T const&>(*ptr);
        }
    }

    template <typename T>
    auto cget() const ->tl::optional<T const&>
    {
        return get();
    }
};