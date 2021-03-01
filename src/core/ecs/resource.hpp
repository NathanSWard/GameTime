#pragma once

#include <memory>
#include <numeric>
#include <unordered_map>
#include <tl/optional.hpp>
#include "util/type_map.hpp"
#include "util/common.hpp"

using system_id_t = std::size_t;

template <typename T, typename Tag>
struct ResourceBase
{
    static_assert(!std::is_reference_v<T>, "Resources cannot be a reference type");
    T* m_ptr = nullptr;

    constexpr ResourceBase(T& value) noexcept : m_ptr(std::addressof(value)) {}

    template <typename U>
    friend constexpr auto make_resource(U&) ->ResourceBase<U, Tag>;
    template <typename U>
    friend constexpr auto make_const_resource(U&) -> ResourceBase<U const, Tag>;

public:
    constexpr ResourceBase(ResourceBase&&) noexcept = default;
    constexpr ResourceBase(ResourceBase const&) noexcept = default;
    constexpr ResourceBase& operator=(ResourceBase&&) noexcept = default;
    constexpr ResourceBase& operator=(ResourceBase const&) noexcept = default;

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

namespace {
    struct ResourceTag {};
    struct LocalTag {};
}

// TODO: Make Resources Thread Safe!
template <typename T>
using Resource = ResourceBase<T, ResourceTag>;

template <typename T>
using Local = ResourceBase<T, LocalTag>;

template <typename T, typename Tag = ResourceTag>
constexpr auto make_resource(T& value) -> ResourceBase<T, Tag>
{
    return ResourceBase<T, Tag>(value);
}

template <typename T, typename Tag = ResourceTag>
constexpr auto make_const_resource(T const& value) -> ResourceBase<T const, Tag>
{
    return ResourceBase<T const, Tag>(value);
}

template <typename T>
constexpr auto make_local_resource(T& value) -> Local<T>
{
    return make_resource<T, LocalTag>(value);
}

template <typename T>
constexpr auto make_const_local_resource(T const& value) -> Local<T const>
{
    return make_const_resource<T, LocalTag>(value);
}

class LocalResources
{
    std::unordered_map<system_id_t, TypeMap> m_local_resources;

public:
    template <typename T, typename... Args>
    auto try_add_local_resource(system_id_t const id, Args&&... args) -> Local<T>
    {
        auto [iter, ok] = m_local_resources.try_emplace(id);
        UNUSED(ok);

        T& local = iter->second.try_add<T>(FWD(args)...);
        return Local(local);
    }

    template <typename T, typename... Args>
    auto set_local_resource(system_id_t const id, Args&&... args) -> Local<T>
    {
        auto [iter, ok] = m_local_resources.try_emplace(id);
        UNUSED(ok);

        T& local = iter->second.set<T>(FWD(args)...);
        return Local(local);
    }

    template <typename T>
    auto remove_local_resource(system_id_t const id) -> std::unique_ptr<T>
    {
        if (auto const iter = m_local_resources.find(id); iter != m_local_resources.end()) {
            auto ptr = iter->second.remove<T>();
            if (iter->second.empty()) { // remove the local resources if 
                m_local_resources.erase(iter);
            }
            return ptr;
        }
        return nullptr;
    }

    template <typename T>
    [[nodiscard]] auto contains_local_resource(system_id_t const id) const -> bool
    {
        if (auto const iter = m_local_resources.find(id); iter != m_local_resources.end()) {
            return iter->second.contains<T>();
        }
        return false;
    }

    template <typename T>
    [[nodiscard]] auto get_local_resource(system_id_t const id) -> tl::optional<Local<T>>
    {
        if (auto const iter = m_local_resources.find(id); iter != m_local_resources.end()) {
            return iter->second.get<T>().map([](T& value) { return make_local_resource(value); });
        }
        return {};
    }

    template <typename T>
    [[nodiscard]] auto get_local_resource(system_id_t const id) const -> tl::optional<Local<T const>>
    {
        if (auto const iter = m_local_resources.find(id); iter != m_local_resources.end()) {
            return iter->second.get<T const>().map([](T const& value) { return make_const_local_resource(value); });
        }
        return {};
    }

    template <typename T>
    [[nodiscard]] auto cget_local_resource(system_id_t const id) const -> tl::optional<Local<T const>>
    {
        return get_local_resource<T>();
    }

    void clear_local_resources(system_id_t const id)
    {
        m_local_resources.erase(id);
    }

    void clear_all_local_resources() noexcept
    {
        m_local_resources.clear();
    }

    [[nodiscard]] auto local_resource_count(system_id_t const id) const noexcept -> tl::optional<std::size_t>
    {
        if (auto const iter = m_local_resources.find(id); iter != m_local_resources.end()) {
            return iter->second.size();
        }
        return {};
    }
};

class Resources
{
    TypeMap m_resources;
    LocalResources m_local_resources;

public:

    // Resources
    template <typename T, typename... Args>
    auto try_add_resource(Args&&... args) -> Resource<T>
    {
        T& resource = m_resources.try_add<T>(FWD(args)...);
        return Resource(resource);
    }

    template <typename T, typename... Args>
    auto set_resource(Args&&... args) -> Resource<T>
    {
        T& resource = m_resources.set<T>(FWD(args)...);
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

    auto clear_resources()
    {
        m_resources.clear();
    }

    [[nodiscard]] auto resource_count() const noexcept -> std::size_t
    {
        return m_resources.size();
    }

    [[nodiscard]] auto local() noexcept -> auto&
    {
        return m_local_resources;
    }

    [[nodiscard]] auto local() const noexcept -> auto const&
    {
        return m_local_resources;
    }
};