#pragma once

#include <entt/entt.hpp>
#include <functional>
#include <util/rng.hpp>
#include <tl/optional.hpp>

class HandleId 
{
    entt::id_type m_type_id;
    std::uint64_t m_id;

public:
    constexpr HandleId(entt::id_type const type_id, std::uint64_t const id) noexcept
        : m_type_id(type_id)
        , m_id(id)
    {
    }

    template <typename T>
    [[nodiscard]] static auto random() -> HandleId
    {
        return HandleId{ entt::type_id<T>().hash(), util::uniform_rand<std::uint64_t>() };
    }

    [[nodiscard]] constexpr auto id() const noexcept -> std::uint64_t { return m_id; }
    [[nodiscard]] constexpr auto type_id() const noexcept { return m_type_id; }

    constexpr auto operator==(HandleId const& rhs) const noexcept -> bool
    {
        return m_type_id == rhs.m_type_id && m_id == rhs.m_id;
    }

    constexpr auto operator!=(HandleId const& rhs) const noexcept -> bool
    {
        return !(*this == rhs);
    }
};

template <>
struct std::hash<HandleId>
{
    auto operator()(HandleId const& id) const noexcept -> std::size_t
    {
        return id.id();
    }
};

class UntypedHandle;

template <typename T>
class Handle 
{
    HandleId m_id;

    constexpr Handle(HandleId id) noexcept : m_id(id) {}
    
    template <typename U>
    friend class Assets;
    friend class UntypedHandle;

public:
    [[nodiscard]] constexpr auto id() const noexcept -> HandleId
    {
        return m_id;
    }

    [[nodiscard]] constexpr auto untyped() const noexcept -> UntypedHandle;
};

class UntypedHandle
{
    HandleId m_id;

    constexpr UntypedHandle(HandleId id) noexcept : m_id(id) {}

    template <typename U>
    friend class Assets;
    template <typename T>
    friend class Handle;

public:
    [[nodiscard]] constexpr auto id() const noexcept -> HandleId
    {
        return m_id;
    }

    template <typename T>
    [[nodiscard]] constexpr auto typed() const noexcept -> tl::optional<Handle<T>>
    {
        if (id().type_id() != entt::type_id<T>().hash()) {
            return {};
        }
        return Handle<T>{ id() };
    }
};

template <typename T>
constexpr auto Handle<T>::untyped() const noexcept -> UntypedHandle
{
    return UntypedHandle{ id() };
}