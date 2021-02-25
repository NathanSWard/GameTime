#pragma once

#include <memory>
#include <new>
#include <tl/optional.hpp>
#include <utility>
#include <util/common.hpp>

template <typename T>
class Rc;

namespace {

    template <typename T>
    struct Rc_storage_base 
    {
        std::aligned_storage_t<sizeof(T), alignof(T)> m_value;
        std::size_t m_weak_count{ 1 };
        std::size_t m_strong_count{ 1 };

    public:
        template <typename... Args>
        constexpr explicit Rc_storage_base(in_place_t, Args&&... args) 
        {
            new(std::addressof(m_value)) T(FWD(args)...);
        }

        constexpr auto weak_count() const noexcept -> std::size_t { return m_weak_count; }
        constexpr auto strong_count() const noexcept -> std::size_t { return m_strong_count; }

        constexpr void inc_strong_count() noexcept { ++m_strong_count; }
        constexpr void inc_weak_count() noexcept { ++m_weak_count; }

        constexpr void dec_weak_count() noexcept 
        {
            if (--m_weak_count == 0 && m_strong_count == 0) {
                delete this;
            }
        }

        constexpr void dec_strong_count() 
        {
            if (--m_strong_count == 0) {
                reinterpret_cast<T&>(m_value).~T();
                if (--m_weak_count == 0) {
                    delete this;
                }
            }
        }

        constexpr auto get_val() noexcept -> T& { return reinterpret_cast<T&>(m_value); }
        constexpr auto get_val() const noexcept -> T const& { return reinterpret_cast<T const&>(m_value); }

        constexpr auto get_ptr() noexcept -> T* { return static_cast<T*>(std::addressof(m_value)); }
        constexpr auto get_ptr() const noexcept -> T const* { return static_cast<T const*>(std::addressof(m_value)); }
    };

} // namespace

template <typename T>
class Weak {
    Rc_storage_base<T>* m_base;

    constexpr explicit Weak(Rc<T> const& rc) noexcept
        : m_base(rc.m_base)
    {
        m_base->inc_weak_count();
    }

    constexpr Weak() noexcept : m_base(nullptr) {}

    friend class Rc<T>;

public:
    constexpr Weak(Weak const& w) noexcept
        : m_base(w.m_base)
    {
        m_base->inc_weak_count();
    }

    constexpr Weak(Weak&& w) noexcept
        : m_base(std::exchange(w.m_base, nullptr))
    {}

    constexpr Weak& operator=(Weak const& w) noexcept 
    {
        m_base = w.m_base;
        m_base->inc_weak_count();
        return *this;
    }

    constexpr Weak& operator=(Weak&& w) noexcept 
    {
        m_base = std::exchange(w.m_base, nullptr);
        return *this;
    }

    ~Weak() 
    {
        if (m_base) {
            m_base->dec_weak_count();
        }
    }

    constexpr auto upgrade() const noexcept -> tl::optional<Rc<T>>
    {
        if (m_base && m_base->strong_count() > 0) {
            return Rc<T>(*this);
        }
        return {};
    }

    constexpr auto ptr_eq(Weak const& other) const noexcept -> bool
    {
        return m_base == other.m_base;
    }
};

template <typename T>
class Rc {
    Rc_storage_base<T>* m_base;

    constexpr explicit Rc(Weak::Weak<T> const& w) noexcept
        : m_base(w.m_base)
    {
        m_base->inc_strong_count();
    }

    explicit Rc(T const* const ptr) noexcept
        : m_base(reinterpret_cast<Rc_storage_base<T>*>(ptr))
    {}

    friend class Weak::Weak<T>;

public:
    template <typename... Args>
    constexpr Rc(in_place_t, Args&&... args)
        : m_base(new Rc_storage_base<T>(in_place, FWD(args)...))
    {}

    template <typename... Args>
    static constexpr auto create(Args&&... args) -> Rc
    {
        return Rc<T>(in_place, FWD(args)...);
    }

    constexpr Rc(Rc const& other) noexcept
        : m_base(other.m_base)
    {
        m_base->inc_strong_count();
    }

    constexpr Rc(Rc&& other) noexcept
        : m_base(std::exchange(other.m_base, nullptr))
    {}

    constexpr Rc& operator=(Rc const& rhs) 
    {
        m_base->dec_strong_count();
        m_base = rhs.m_base;
        m_base->inc_strong_count();
        return *this;
    }

    constexpr Rc& operator=(Rc&& rhs) noexcept 
    {
        m_base->dec_strong_count();
        m_base = std::exchange(rhs.m_base, nullptr);
        return *this;
    }

    ~Rc() 
    {
        if (m_base) {
            m_base->dec_strong_count();
        }
    }

    constexpr auto operator*() noexcept -> T& { return m_base->get_val(); }
    constexpr auto operator*() const noexcept -> T const& { return m_base->get_val(); }
    constexpr auto operator->() noexcept -> T* { return m_base->get_ptr(); }
    constexpr auto operator->() const noexcept -> T const* { return m_base->get_ptr(); }

    constexpr auto downgrade() const noexcept -> Weak<T>
    {
        return Weak<T>(*this);
    }

    constexpr auto ptr_eq(Rc const& other) const noexcept -> bool
    {
        return m_base == other.m_base;
    }

    constexpr auto strong_count() const noexcept -> std::size_t 
    {
        return m_base->strong_count();
    }

    constexpr auto weak_count() const noexcept -> std::size_t 
    {
        return m_base->weak_count() - 1;
    }
};