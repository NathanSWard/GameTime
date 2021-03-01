#pragma once

#include <type_traits>
#include <utility>

namespace {

    template <typename T>
    requires (!std::is_trivially_destructible_v<T>)
        constexpr void invoke_destructor(void* const ptr)
    {
        static_cast<T*>(ptr)->~T();
    }

    template <typename T>
    requires (std::is_trivially_destructible_v<T>)
        constexpr void invoke_destructor(void* const) {}

} // namespace

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
        T* ptr = new T(FWD(args)...);
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