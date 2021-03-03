#pragma once

#include <util/common.hpp>
#include <ranges>
#include <type_traits>

struct ChainSentinel {};

template <std::ranges::view A, std::ranges::view B>
class ChainIterator
{
    using a_iter_t = std::ranges::iterator_t<A>;
    using a_sent_t = std::ranges::sentinel_t<A>;
    using a_val_t = std::ranges::range_value_t<A>;

    using b_iter_t = std::ranges::iterator_t<B>;
    using b_sent_t = std::ranges::sentinel_t<B>;
    using b_val_t = std::ranges::range_value_t<B>;

    static_assert(std::is_same_v<a_val_t, b_val_t>);

    a_iter_t m_a_iter;
    a_sent_t m_a_sent;

    b_iter_t m_b_iter;
    b_sent_t m_b_sent;

public:
    template <typename T, typename U>
    constexpr ChainIterator(T&& t, U&& u) noexcept
        : m_a_iter(std::ranges::begin(t))
        , m_a_sent(std::ranges::end(t))
        , m_b_iter(std::ranges::begin(u))
        , m_b_sent(std::ranges::end(u))
    {}

    constexpr auto operator==(ChainSentinel) const noexcept -> bool
    {
        return m_a_iter == m_a_sent ? m_b_iter == m_b_sent : false;
    }

    constexpr auto operator!=(ChainSentinel) const noexcept -> bool
    {
        return !(*this == ChainSentinel{});
    }

    constexpr ChainIterator& operator++() noexcept
    {
        if (m_a_iter != m_a_sent) {
            ++m_a_iter;
        }
        else {
            ++m_b_iter;
        }
        
        return *this;
    }

    [[nodiscard]] constexpr auto operator*() const noexcept -> decltype(auto)
    {
        if (m_a_iter != m_a_sent) {
            return *m_a_iter;
        }
        else {
            return *m_b_iter;
        }
    }

    [[nodiscard]] constexpr auto operator*() noexcept -> decltype(auto)
    {
        if (m_a_iter != m_a_sent) {
            return *m_a_iter;
        }
        else {
            return *m_b_iter;
        }
    }
};

template <std::ranges::view A, std::ranges::view B>
class chain_view : public std::ranges::view_interface<chain_view<A, B>>
{
    A m_a_view;
    B m_b_view;

public:
    constexpr chain_view() noexcept = default;

    constexpr chain_view(A a_view, B b_view) noexcept 
        : m_a_view(MOV(a_view))
        , m_b_view(MOV(b_view))
    {}

    constexpr chain_view(chain_view&&) = default;
    constexpr chain_view(chain_view const&) = default;
    constexpr chain_view& operator=(chain_view&&) = default;
    constexpr chain_view& operator=(chain_view const&) = default;

    constexpr auto begin() const noexcept
    { 
        return ChainIterator<A, B>{m_a_view, m_b_view};
    }

    constexpr auto begin() noexcept
    {
        return ChainIterator<A, B>{m_a_view, m_b_view};
    }

    constexpr auto end() const noexcept { return ChainSentinel{}; }

    constexpr auto size() const noexcept
        requires(std::ranges::sized_range<A> && std::ranges::sized_range<B>)
    {
        return m_a_view.size() + m_b_view.size();
    }
};

template <typename A, typename B>
constexpr auto chain(A&& a, B&& b)
{
    return chain_view(std::views::all(a), std::views::all(b));
}

