#pragma once

#include <compare>
#include <functional>
#include <util/common.hpp>
#include <util/uuid.hpp>
#include <tl/optional.hpp>

class HandleId 
{
    Uuid m_uuid;

public:
    constexpr HandleId(Uuid const uuid) noexcept
        : m_uuid(uuid)
    {}

    [[nodiscard]] constexpr auto uuid() const noexcept -> Uuid
    {
        return m_uuid;
    }

    template <typename T>
    [[nodiscard]] static auto create() -> HandleId
    {
        return HandleId{ Uuid::create() };
    }

    constexpr auto operator<=>(HandleId const&) const noexcept = default;
};

template <>
struct std::hash<HandleId>
{
    auto operator()(HandleId const& id) const noexcept -> std::size_t
    {
        return std::hash<Uuid>{}(id.uuid());
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
    [[nodiscard]] auto id() const noexcept -> HandleId { return m_id; }
    [[nodiscard]] constexpr auto untyped() const noexcept -> UntypedHandle;
};

class UntypedHandle
{
    HandleId m_id;
    type_id_t m_type_id;

    constexpr UntypedHandle(HandleId id, type_id_t const type_id) noexcept 
        : m_id(id)
        , m_type_id(type_id)
    {}

    template <typename U>
    friend class Assets;
    template <typename T>
    friend class Handle;

public:
    [[nodiscard]] auto id() const noexcept -> HandleId { return m_id; }
    [[nodiscard]] auto type() const noexcept -> type_id_t { return m_type_id; }

    template <typename T>
    [[nodiscard]] auto is_handle() const noexcept -> bool
    {
        return m_type_id == type_id<T>();
    }

    template <typename T>
    [[nodiscard]] constexpr auto typed() const noexcept -> tl::optional<Handle<T>>
    {
        if (m_type_id != type_id<T>()) {
            return {};
        }
        return Handle<T>{ id() };
    }
};

template <typename T>
constexpr auto Handle<T>::untyped() const noexcept -> UntypedHandle
{
    return UntypedHandle{ m_id, type_id<T>() };
}

template <typename T>
constexpr auto operator==(Handle<T> const& handle, UntypedHandle const uhandle) noexcept -> bool
{
    return uhandle.type() == type_id<T>() && uhandle.id() == handle.id();
}

template <typename T>
constexpr auto operator==(UntypedHandle const uhandle, Handle<T> const& handle) noexcept -> bool
{
    return handle == uhandle;
}