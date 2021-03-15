#pragma once

#include <bit>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <string_view>
#include <util/common.hpp>
#include <util/rng.hpp>

template <typename T>
class Handle;

class UntypedHandle;

struct AssetPathId {
    std::size_t id = 0;

    constexpr auto operator==(AssetPathId const& other) const noexcept -> bool
    {
        return id == other.id;
    }

    constexpr auto operator!=(AssetPathId const& other) const noexcept -> bool
    {
        return !(*this == other);
    }
};

template <>
struct std::hash<AssetPathId>
{
    auto operator()(AssetPathId const& apid) const noexcept -> std::size_t
    {
        return apid.id;
    }
};

class HandleId
{
public:
    struct Uid {
        std::uint64_t id;
        type_id_t type_id;
    };

private:
    union {
        AssetPathId m_path_id{};
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
        return HandleId(AssetPathId { .id = hash });
    }

    template <typename T>
    [[nodiscard]] static auto random() -> HandleId
    {
        return HandleId(Uid{
            .id = util::uniform_rand<std::uint64_t>(),
            .type_id = type_id<T>()
            });
    }

    [[nodiscard]] constexpr auto operator==(HandleId const& other) const noexcept -> bool
    {
        if (m_is_path_id != other.m_is_path_id) {
            return false;
        }

        if (m_is_path_id) {
            return m_path_id == other.m_path_id;
        }
        else {
            return m_uid.id == other.m_uid.id && m_uid.type_id == other.m_uid.type_id;
        }
    }

    [[nodiscard]] constexpr auto operator!=(HandleId const& other) const noexcept -> bool
    {
        return !(*this == other);
    }
};

namespace {
    [[nodiscard]] CONSTEXPR_BIT_CAST auto to_bytes(HandleId const& id) noexcept -> std::array<char, sizeof(HandleId)>
    {
        return bit_cast<std::array<char, sizeof(HandleId)>>(id);
    }
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

// format specifiers

template <>
struct fmt::formatter<AssetPathId> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(AssetPathId const& apid, Ctx& ctx) {
        return format_to(ctx.out(), "AssetPathId(id: {})", apid.id);
    }
};

template <>
struct fmt::formatter<HandleId> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(HandleId const& hid, Ctx& ctx) {
        if (hid.m_is_path_id) {
            return format_to(ctx.out(), "HandleId::{}", hid.m_path_id);
        }
        else {
            return format_to(ctx.out(), "HandleId::Uid(id: {}, type_id: {})", hid.m_uid.id, hid.m_uid.type_id);
        }
    }
};