#pragma once

#include "util/common.hpp"
#include <entt/core/type_info.hpp>
#include <tl/optional.hpp>
#include <type_traits>
#include <unordered_map>

namespace internal {

    template <typename T>
    requires (!std::is_trivially_destructible_v<T>)
    constexpr void invoke_destructor(void* const ptr)
    {
        static_cast<T*>(ptr)->~T();
    }

    template <typename T>
    requires (std::is_trivially_destructible_v<T>)
    constexpr void invoke_destructor(void* const) {}

    class void_ptr {
        using deleter_t = void(*)(void*);
        void* m_data = nullptr;
        deleter_t m_deleter = nullptr;

        constexpr void_ptr(void* const data, deleter_t const deleter) noexcept
            : m_data(data)
            , m_deleter(deleter)
        {
        }

        constexpr void destory()
        {
            if (m_data) {
                m_deleter(m_data);
            }
        }

    public:
        template <typename T, typename... Args>
        static constexpr auto create(Args&&... args) noexcept -> void_ptr 
        {
            T* ptr = new T(std::forward<Args>(args)...);
            return void_ptr(static_cast<void*>(ptr), invoke_destructor<T>);
        }

        constexpr void* take() noexcept 
        {
            auto const ptr = m_data;
            m_data = nullptr;
            m_deleter = nullptr;
            return ptr;
        }

        constexpr auto data() noexcept -> void* { return m_data; }
        constexpr auto data() const noexcept -> void const* { return m_data; }
        constexpr auto cdata() const noexcept -> void const* { return m_data; }

        constexpr void_ptr(void_ptr&& other) noexcept 
            : m_data(std::exchange(other.m_data, nullptr))
            , m_deleter(std::exchange(other.m_deleter, nullptr))
        {
        }

        constexpr void_ptr& operator=(void_ptr&& other) noexcept
        {
            destory();
            m_data = std::exchange(other.m_data, nullptr);
            m_deleter = std::exchange(other.m_deleter, nullptr);
            return *this;
        }

        void_ptr(void_ptr const&) = delete;
        void_ptr& operator=(void_ptr const&) = delete;

        ~void_ptr() { destory(); }
    };

} // namespace internal

class TypeMap {
    std::unordered_map<entt::id_type, internal::void_ptr> m_map;

public:

    [[nodiscard]] auto size() const noexcept -> std::size_t
    {
        return m_map.size();
    }

    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return m_map.empty();
    }

    template <typename T, typename... Args>
    auto insert(Args&&... args) -> T&
    {
        auto const [iter, ok] = m_map.insert_or_assign(entt::type_id<T>().hash(), internal::void_ptr::template create<T>(FWD(args)...));
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
        if (auto const iter = m_map.find(entt::type_id<T>().hash()); iter == m_map.end()) {
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
        if (auto const iter = m_map.find(entt::type_id<T>().hash()); iter == m_map.end()) {
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
        if (auto const iter = m_map.find(entt::type_id<T>().hash()); iter == m_map.end()) {
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