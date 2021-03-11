#pragma once

#include <bit>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <util/common.hpp>
#include <tl/optional.hpp>
#include <util/rng.hpp>
#include <util/sync/mpmc.hpp>
#include <debug/debug.hpp>

template <typename T>
class Handle;

class UntypedHandle;

using AssetPathId = std::uint64_t;

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
    [[nodiscard]] constexpr auto to_bytes(HandleId const& id) noexcept -> std::array<char, sizeof(HandleId)>
    {
        return std::bit_cast<std::array<char, sizeof(HandleId)>>(id);
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

namespace detail {

    struct RefChange
    {
        enum Type : bool { Increment, Decrement } type = Increment;
        HandleId id = HandleId(AssetPathId(0));

        [[nodiscard]] constexpr static auto increment(HandleId const id) noexcept -> RefChange
        {
            return RefChange{ .type = Increment, .id = id };
        }

        [[nodiscard]] constexpr static auto decrement(HandleId const id) noexcept -> RefChange
        {
            return RefChange{ .type = Decrement, .id = id };
        }
    };

}

template <typename T>
class Handle 
{
    HandleId m_id;
    tl::optional<Sender<detail::RefChange>> m_sender;

    constexpr Handle(HandleId const id) noexcept : m_id(id) {}
    Handle(HandleId const id, Sender<detail::RefChange>&& sender) noexcept
        : m_id(id)
        , m_sender(MOV(sender))
    {}

public:
    constexpr Handle(Handle&&) noexcept = default;
    constexpr Handle& operator=(Handle&&) noexcept = default;

    ~Handle()
    {
        if (m_sender.has_value()) {
            m_sender->send(detail::RefChange::decrement(m_id));
        }
    }

    // implicit conversion to a HandleId
    constexpr operator HandleId() const noexcept { return m_id; }

    [[nodiscard]] static auto weak(HandleId const id) -> Handle
    {
        return Handle(id);
    }

    [[nodiscard]] static auto strong(HandleId const id, Sender<detail::RefChange> sender) -> Handle
    {
        sender.send(detail::RefChange::increment(id));
        return Handle(id, MOV(sender));
    }

    [[nodiscard]] auto id() const noexcept -> HandleId { return m_id; }

    [[nodiscard]] constexpr auto is_weak() const noexcept -> bool { return !m_sender.has_value(); }
    [[nodiscard]] constexpr auto is_strong() const noexcept -> bool { return m_sender.has_value(); }

    [[nodiscard]] auto copy() const noexcept -> Handle<T>
    {
        if (is_strong()) {
            return Handle<T>::strong(m_id, *m_sender);
        }
        else {
            returrn Handle<T>::weak(m_id);
        }
    }

    [[nodiscard]] auto copy_weak() const noexcept -> Handle<T>
    {
        return Handle<T>::weak(m_id);
    }

    [[nodiscard]] auto untyped() const noexcept -> UntypedHandle;
    [[nodiscard]] constexpr auto weak_untyped() const noexcept -> UntypedHandle;
};

class UntypedHandle
{
    HandleId m_id;
    tl::optional<Sender<detail::RefChange>> m_sender;

    constexpr UntypedHandle(HandleId const id) noexcept
        : m_id(id)
    {}

    UntypedHandle(HandleId const id, Sender<detail::RefChange>&& sender) noexcept
        : m_id(id)
        , m_sender(MOV(sender))
    {}

public:
    constexpr UntypedHandle(UntypedHandle&&) noexcept = default;
    constexpr UntypedHandle& operator=(UntypedHandle&&) noexcept = default;

    ~UntypedHandle()
    {
        if (m_sender.has_value()) {
            m_sender->send(detail::RefChange::decrement());
        }
    }

    // implicit conversion to a HandleId
    constexpr operator HandleId() const noexcept { return m_id; }

    [[nodiscard]] static auto weak(HandleId const id) -> UntypedHandle
    {
        return UntypedHandle(id);
    }

    [[nodiscard]] static auto strong(HandleId const id, Sender<detail::RefChange> sender) -> UntypedHandle
    {
        sender.send(detail::RefChange::increment(id));
        return UntypedHandle(id, MOV(sender));
    }

    [[nodiscard]] auto id() const noexcept -> HandleId { return m_id; }

    [[nodiscard]] constexpr auto is_weak() const noexcept -> bool { return !m_sender.has_value(); }
    [[nodiscard]] constexpr auto is_strong() const noexcept -> bool { return m_sender.has_value(); }

    [[nodiscard]] auto copy() const noexcept -> UntypedHandle
    {
        if (is_strong()) {
            return UntypedHandle::strong(m_id, *m_sender);
        }
        else {
            return UntypedHandle::weak(m_id);
        }
    }

    [[nodiscard]] auto copy_weak() const noexcept -> UntypedHandle
    {
        return UntypedHandle::weak(m_id);
    }

    template <typename T>
    [[nodiscard]] constexpr auto typed() && -> Handle<T>
    {
        if (!m_id.m_is_path_id) {
            if (m_id.m_uid.type_id != type_id<T>()) [[unlikely]] {
                PANIC("Attempted to convert UntypedHandle to invalid type.");
            }
        }

        auto sender = m_sender.take();
        if (sender) {
            return Handle<T>::strong(m_id, *MOV(sender));
        }
        else {
            return Handle<T>::weak(m_id);
        }
    }
};

template <typename T>
[[nodiscard]] auto Handle<T>::untyped() const noexcept -> UntypedHandle
{
    if (is_strong()) {
        return UntypedHandle::strong(m_id, *m_sender);
    }
    else {
        return UntypedHandle::weak(m_id);
    }
}

template <typename T>
[[nodiscard]] constexpr auto Handle<T>::weak_untyped() const noexcept -> UntypedHandle
{
    return UntypedHandle(m_id);
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