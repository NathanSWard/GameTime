#pragma once

#include <shared_mutex>
#include "mutex.hpp"

template <typename T>
class RwLock;

template <typename T>
class WriteGuard;

template <typename T>
class ReadGuard;

template <typename T>
class WriteGuard 
{
    RwLock<T>* m_rwlock;

    template <typename U>
    friend class RwLock;
    
    constexpr explicit WriteGuard(RwLock<T>& rwlock) noexcept
        : m_rwlock(std::addressof(rwlock))
    {}

public:
    constexpr WriteGuard(WriteGuard&& other) noexcept
        : m_rwlock(std::exchange(other.m_rwlock, nullptr))
    {}

    constexpr WriteGuard& operator=(WriteGuard&& other) noexcept
    {
        m_rwlock = std::exchange(other.m_rwlock, nullptr);
        return *this;
    }

    WriteGuard(WriteGuard const&) = delete;
    WriteGuard& operator=(WriteGuard const&) = delete;

    ~WriteGuard() noexcept
    {
        if (m_rwlock) {
            m_rwlock->m_mutex.unlock();
        }
    }

    [[nodiscard]] constexpr auto operator*() noexcept -> T& { return m_rwlock->m_value; }
    [[nodiscard]] constexpr auto operator*() const noexcept -> T const& { return m_rwlock->m_value; }
    [[nodiscard]] constexpr auto operator->() noexcept -> T* { return std::addressof(m_rwlock->m_value); }
    [[nodiscard]] constexpr auto operator->() const noexcept -> T const* { return std::addressof(m_rwlock->m_value); }
};

template <typename T>
class ReadGuard 
{
    RwLock<T> const* m_rwlock;

    template <typename U>
    friend class RwLock;

    constexpr explicit ReadGuard(RwLock<T> const& rwlock) noexcept
        : m_rwlock(std::addressof(rwlock))
    {}

public:
    constexpr ReadGuard(ReadGuard&& other) noexcept
        : m_rwlock(std::exchange(other.m_rwlock, nullptr))
    {}

    constexpr ReadGuard& operator=(ReadGuard&& other) noexcept
    {
        m_rwlock = std::exchange(other.m_rwlock, nullptr);
        return *this;
    }

    ReadGuard(ReadGuard const&) = delete;
    ReadGuard& operator=(ReadGuard const&) = delete;

    ~ReadGuard() noexcept
    {
        if (m_rwlock) {
            m_rwlock->m_mutex.unlock_shared();
        }
    }

    [[nodiscard]] constexpr auto operator*() const noexcept -> T const& { return m_rwlock->m_value; }
    [[nodiscard]] constexpr auto operator->() const noexcept -> T const* { return std::addressof(m_rwlock->m_value); }
};

template <typename T>
class RwLock
{
    T m_value;
    std::shared_mutex mutable m_mutex;

    template <typename U>
    friend class WriteGuard;
    template <typename U>
    friend class ReadGuard;

public:
    constexpr RwLock() = default;

    template <typename... Args>
    constexpr explicit RwLock(in_place_t, Args&&... args)
        : m_value(FWD(args)...)
    {}

    template <typename... Args>
    [[nodiscard]] static auto create(Args&&... args) -> RwLock
    {
        return RwLock<T>(in_place, FWD(args)...);
    }

    constexpr RwLock(RwLock&& other) noexcept
        : m_value(
#ifdef ISDEBUG
            [&] {
                DEBUG_ASSERT(other.m_mutex.try_lock());
                other.m_mutex.unlock();
                return MOV(other.m_value);
            }()
#else 
            MOV(other.m_value)
#endif // ISDEBUG
        )
    {}

    RwLock& operator=(RwLock&&) = delete;
    RwLock(RwLock const&) = delete;
    RwLock& operator=(RwLock const&) = delete;

    [[nodiscard]] constexpr auto write() noexcept -> WriteGuard<T>
    {
        m_mutex.lock();
        return WriteGuard<T>{ const_cast<RwLock&>(*this) };
    }

    [[nodiscard]] constexpr auto try_write() noexcept -> tl::optional<WriteGuard<T>>
    {
        if (!m_mutex.try_lock()) {
            return {};
        }
        return WriteGuard<T>{ const_cast<RwLock&>(*this) };
    }

    [[nodiscard]] constexpr auto read() const noexcept -> ReadGuard<T>
    {
        m_mutex.lock_shared();
        return ReadGuard<T>{ *this };
    }

    [[nodiscard]] constexpr auto try_read() const noexcept -> tl::optional<ReadGuard<T>>
    {
        if (!m_mutex.try_lock_shared()) {
            return {};
        }
        return ReadGuard<T>{ *this };
    }
};