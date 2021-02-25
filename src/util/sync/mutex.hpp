#pragma once

#include <mutex>
#include <debug/debug.hpp>
#include <util/common.hpp>
#include <tl/optional.hpp>

template <typename T>
class Mutex;

template <typename T>
class MutexGuard;

template <typename T>
class MutexGuard 
{
    Mutex<T>* m_mutex;

    template <typename U>
    friend class Mutex;

    constexpr explicit MutexGuard(Mutex<T>& mutex) noexcept
        : m_mutex(std::addressof(mutex))
    {}

public:
    constexpr MutexGuard(MutexGuard&& other) noexcept
        : m_mutex(std::exchange(other.m_mutex, nullptr))
    {}

    constexpr MutexGuard& operator=(MutexGuard&& other) noexcept 
    {
        m_mutex = std::exchange(other.m_mutex, nullptr);
    }

    MutexGuard(MutexGuard const&) = delete;
    MutexGuard& operator=(MutexGuard const&) = delete;

    ~MutexGuard() noexcept 
    {
        if (m_mutex) {
            m_mutex->m_mutex.unlock();
        }
    }

    [[nodiscard]] constexpr auto operator*() noexcept -> T& { return m_mutex->m_value; }
    [[nodiscard]] constexpr auto operator*() const noexcept -> T const& { return m_mutex->m_value; }
};

template <typename T>
class Mutex 
{
    T m_value;
    std::mutex m_mutex;

    template <typename U>
    friend class MutexGuard;

public:
    template <typename... Args>
    constexpr explicit Mutex(in_place_t, Args&&... args)
        : m_value(FWD(args)...)
    {}

    constexpr Mutex(Mutex&& other) noexcept
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

    Mutex& operator=(Mutex&&) = delete;
    Mutex(Mutex const&) = delete;
    Mutex& operator=(Mutex const&) = delete;

    [[nodiscard]] constexpr auto lock() noexcept -> MutexGuard<T> 
    {
        m_mutex.lock();
        return MutexGuard<T>{ *this };
    }

    [[nodiscard]] constexpr auto try_lock() noexcept -> tl::optional<MutexGuard<T>>
    {
        if (!m_mutex.try_lock()) {
            return {};
        }
        return MutexGuard<T>{ *this };
    }
};

template <typename T, typename... Args>
constexpr auto make_mutex(Args&&... args) -> Mutex<T>
{
    return Mutex<T>{ in_place, FWD(args)... };
}