#pragma once

#include <bit>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <util/common.hpp>
#include <tl/optional.hpp>
#include <util/rng.hpp>
#include <debug/debug.hpp>

template <typename T>
class Handle;

class UntypedHandle;

using AssetPathId = std::uint64_t;

class HandleId 
{
public:
    struct Uid {
        std::uint64_t id{};
        type_id_t   type_id{};
    };

private:
    union {
        AssetPathId m_path_id;
        Uid m_uid;
    };
    bool m_is_path_id = false;

    template <typename T>
    friend class Handle;
    friend class UntypedHandle;
    friend struct std::hash<HandleId>;
    friend class AssetServer;
    friend struct fmt::formatter<HandleId>;

public:
    constexpr explicit HandleId(AssetPathId const path_id) noexcept
        : m_path_id(path_id)
        , m_is_path_id(true)
    {}

    constexpr HandleId(Uid const uid) noexcept
        : m_uid(uid)
        , m_is_path_id(false)
    {}

    constexpr HandleId(HandleId&&) noexcept = default;
    constexpr HandleId(HandleId const&) noexcept = default;
    constexpr HandleId& operator=(HandleId&&) noexcept = default;
    constexpr HandleId& operator=(HandleId const&) noexcept = default;

    [[nodiscard]] static auto from_path(std::filesystem::path const& path) -> HandleId
    {
        auto const hash = std::filesystem::hash_value(path);
        return HandleId(static_cast<AssetPathId>(hash));
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

template <typename T>
class Handle 
{
    HandleId m_id;

public:
    constexpr Handle(HandleId id) noexcept : m_id(id) {}

    constexpr Handle(Handle&&) noexcept = default;
    constexpr Handle& operator=(Handle&&) noexcept = default;

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

// format specifiers

template <>
struct fmt::formatter<HandleId> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(HandleId const& hid, Ctx& ctx) {
        if (hid.m_is_path_id) {
            return format_to(ctx.out(), "HandleId::AssetPathId({})", hid.m_path_id);
        }
        else {
            return format_to(ctx.out(), "HandleId::Uid(id: {}, type_id: {})", hid.m_uid.id, hid.m_uid.type_id);
        }
    }
};