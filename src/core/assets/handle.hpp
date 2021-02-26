#pragma once

#include <bit>
#include <functional>
#include <util/common.hpp>
#include <tl/optional.hpp>
#include <util/rng.hpp>
#include <debug/debug.hpp>

template <typename T>
class Handle;

class UntypedHandle;

class HandleId 
{
public:
    struct Uid {
        std::uint64_t id{};
        type_id_t   type_id{};
    };
    using path_id_t = std::uint64_t;

private:
    union {
        path_id_t m_path_id;
        Uid m_uid;
    };
    bool m_is_path_id = false;

    constexpr explicit HandleId(path_id_t const path_id) noexcept
        : m_path_id(path_id)
        , m_is_path_id(true)
    {}

    constexpr HandleId(Uid const uid) noexcept
        : m_uid(uid)
        , m_is_path_id(false)
    {}

    template <typename T>
    friend class Handle;
    friend class UntypedHandle;
    friend struct std::hash<HandleId>;

public:
    constexpr HandleId(HandleId&&) noexcept = default;
    constexpr HandleId(HandleId const&) noexcept = default;
    constexpr HandleId& operator=(HandleId&&) noexcept = default;
    constexpr HandleId& operator=(HandleId const&) noexcept = default;

    [[nodiscard]] constexpr static auto from_path(std::string_view const path) -> HandleId
    {
        return HandleId(static_cast<std::uint64_t>(std::hash<std::string_view>{}(path)));
    }

    template <typename T>
    [[nodiscard]] static auto random() -> HandleId
    {
        return HandleId(Uid{
            .id = util::uniform_rand<std::uint64_t>(),
            .type_id = type_id<T>()
            });
    }
};

namespace {
    [[nodiscard]] constexpr auto to_bytes(HandleId const& id) noexcept -> std::array<char, sizeof(HandleId)>
    {
        return std::bit_cast<std::array<char, sizeof(HandleId)>>(id);
    }
} // namespace

[[nodiscard]] constexpr auto operator==(HandleId const& lhs, HandleId const& rhs) noexcept -> bool
{
    return to_bytes(lhs) == to_bytes(rhs);
}

[[nodiscard]] constexpr auto operator!=(HandleId const& lhs, HandleId const& rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <>
struct std::hash<HandleId>
{
    auto operator()(HandleId const& id) const noexcept -> std::size_t
    {
        auto bytes = to_bytes(id);
        auto str = std::string_view{ bytes.data(), bytes.size() };
        return std::hash<std::string_view>{}(str);
    }
};

class UntypedHandle;

template <typename T>
class Handle 
{
    HandleId m_id;

public:
    constexpr Handle(HandleId id) noexcept : m_id(id) {}

    [[nodiscard]] auto id() const noexcept -> HandleId { return m_id; }
    [[nodiscard]] constexpr auto untyped() const noexcept -> UntypedHandle;
};

class UntypedHandle
{
    HandleId m_id;

public:
    constexpr UntypedHandle(HandleId id) noexcept
        : m_id(id)
    {}

    [[nodiscard]] auto id() const noexcept -> HandleId { return m_id; }

    template <typename T>
    [[nodiscard]] constexpr auto typed() const noexcept -> tl::optional<Handle<T>>
    {
        if (m_id.m_is_path_id) {
            return Handle<T>{ m_id };
        }
        else { // is uid
            if (m_id.m_uid.type_id == type_id<T>()) {
                return Handle<T>{ m_id };
            }
        }
        return {};
    }
};

template <typename T>
[[nodiscard]] constexpr auto Handle<T>::untyped() const noexcept -> UntypedHandle
{
    return UntypedHandle{ m_id };
}