#pragma once

#include <type_traits>
#include <utility>

namespace {

    template <typename T>
    constexpr void delete_data(void* const ptr)
    {
        delete static_cast<T*>(ptr);
    }

} // namespace

class void_ptr {
    using deleter_t = void(*)(void*);
    void* m_data = nullptr;
    deleter_t m_deleter = nullptr;

    constexpr void destory()
    {
        if (m_data) {
            m_deleter(m_data);
        }
    }

public:
    template <typename T, typename... Args>
    constexpr void_ptr(in_place_type_t<T>, Args&&... args)
        : m_data(new T(FWD(args)...))
        , m_deleter(delete_data<T>)
    {}

    template <typename T, typename... Args>
    static constexpr auto create(Args&&... args) noexcept -> void_ptr
    {
        return void_ptr(in_place_type<T>, FWD(args)...);
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