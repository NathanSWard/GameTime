#pragma once

#include <core/assets/handle_id.hpp>
#include <core/assets/ref_change.hpp>
#include <util/common.hpp>
#include <tl/optional.hpp>
#include <util/sync/mpmc.hpp>
#include <debug/debug.hpp>

template <typename T>
class Handle 
{
    HandleId m_id;
    tl::optional<Sender<RefChange>> m_sender;

    constexpr Handle(HandleId const id) noexcept 
        : m_id(id) 
    {}

    Handle(HandleId const id, Sender<RefChange>&& sender) noexcept
        : m_id(id)
        , m_sender(MOV(sender))
    {}

public:
    Handle(Handle&& other) noexcept
        : m_id(other.m_id)
        , m_sender(other.m_sender.take())
    {}

    Handle& operator=(Handle&& other) noexcept
    {
        m_id = other.m_id;

        if (m_sender.has_value()) {
            m_sender->send(RefChange::decrement(m_id));
        }
        m_sender = other.m_sender.take();
        return *this;
    }

    ~Handle()
    {
        if (m_sender.has_value()) {
            m_sender->send(RefChange::decrement(m_id));
        }
    }

    // implicit conversion to a HandleId
    constexpr operator HandleId() const noexcept { return m_id; }

    [[nodiscard]] static auto random() -> Handle
    {
        return Handle();
    }

    [[nodiscard]] static auto weak(HandleId const id) -> Handle
    {
        return Handle(id);
    }

    [[nodiscard]] static auto strong(HandleId const id, Sender<RefChange> sender) -> Handle
    {
        sender.send(RefChange::increment(id));
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
            return Handle<T>::weak(m_id);
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
    tl::optional<Sender<RefChange>> m_sender;

    constexpr UntypedHandle(HandleId const id) noexcept
        : m_id(id)
    {}

    UntypedHandle(HandleId const id, Sender<RefChange>&& sender) noexcept
        : m_id(id)
        , m_sender(MOV(sender))
    {}

public:
    UntypedHandle(UntypedHandle&& other) noexcept
        : m_id(other.m_id)
        , m_sender(other.m_sender.take())
    {}

    UntypedHandle& operator=(UntypedHandle&& other) noexcept
    {
        m_id = other.m_id;

        if (m_sender.has_value()) {
            m_sender->send(RefChange::decrement(m_id));
        }
        m_sender = other.m_sender.take();
        return *this;
    }

    ~UntypedHandle()
    {
        if (m_sender.has_value()) {
            m_sender->send(RefChange::decrement(m_id));
        }
    }

    // implicit conversion to a HandleId
    constexpr operator HandleId() const noexcept { return m_id; }

    [[nodiscard]] static auto weak(HandleId const id) -> UntypedHandle
    {
        return UntypedHandle(id);
    }

    [[nodiscard]] static auto strong(HandleId const id, Sender<RefChange> sender) -> UntypedHandle
    {
        sender.send(RefChange::increment(id));
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