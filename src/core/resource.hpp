#pragma once

#include <memory>
#include <tl/optional.hpp>
#include "type_map.hpp"

template <typename T>
struct Resource 
{
    static_assert(!std::is_reference_v<T>, "Resources cannot be a reference type");
    T* m_ptr = nullptr;

    constexpr Resource(T& value) noexcept : m_ptr(std::addressof(value)) {}

    template <typename U>
    friend constexpr auto make_resource(U&) -> Resource<U>;
    template <typename U>
    friend constexpr auto make_const_resource(U&)->Resource<U const>;

public:

    constexpr Resource(Resource&&) noexcept = default;
    constexpr Resource(Resource const&) noexcept = default;
    constexpr Resource& operator=(Resource&&) noexcept = default;
    constexpr Resource& operator=(Resource const&) noexcept = default;

    [[nodiscard]] constexpr auto operator*() noexcept -> T&
    {
        return *m_ptr;
    }

    [[nodiscard]] constexpr auto operator*() const noexcept -> T const&
    {
        return *m_ptr;
    }

    [[nodiscard]] constexpr auto operator->() noexcept -> T*
    {
        return m_ptr;
    }

    [[nodiscard]] constexpr auto operator->() const noexcept -> T const*
    {
        return m_ptr;
    }

};

template <typename T>
constexpr auto make_resource(T& value) -> Resource<T>
{
    return Resource(value);
}

template <typename T>
constexpr auto make_const_resource(T const& value) -> Resource<T const>
{
    return Resource(value);
}

class ResourceManager
{
    TypeMap m_resources;

public:
    
    template <typename T, typename... Args>
    auto add_resource(Args&&... args) -> Resource<T>
    {
        T& resource = m_resources.insert<T>(std::forward<Args>(args)...);
        return Resource(resource);
    }

    template <typename T>
    auto remove_resource() -> std::unique_ptr<T>
    {
        return m_resources.remove<T>();
    }

    template <typename T>
    [[nodiscard]] auto contains_resource() -> bool
    {
        return m_resources.contains<T>();
    }

    template <typename T>
    [[nodiscard]] auto get_resource() -> tl::optional<Resource<T>>
    {
        return m_resources.get<T>().map([](T& value) { return make_resource(value); });
    }

    template <typename T>
    [[nodiscard]] auto get_resource() const -> tl::optional<Resource<T const>>
    {
        return m_resources.get<T const>().map([](T const& value) { return make_const_resource(value); });
    }

    template <typename T>
    [[nodiscard]] auto cget_resource() const -> tl::optional<Resource<T const>>
    {
        return get_resource<T>();
    }
};