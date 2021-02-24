#pragma once

template <typename... Cs>
struct With {};

template <typename... Cs>
struct Without {};

struct View {};
struct Group {};

template <typename W, typename WO = Without<>, typename VG = View>
struct Query;

template <typename... Ws, typename... WOs>
class Query<With<Ws...>, Without<WOs...>, View>
{
    using base_t = entt::view<entt::exclude_t<WOs...>, Ws...>;
    base_t m_repr;

public:
    template <typename T>
    requires (!std::is_same_v<std::remove_cvref_t<T>, Query>)
        constexpr Query(T&& repr) noexcept : m_repr(FWD(repr)) {}

    auto begin() { return m_repr.begin(); }
    auto end() { return m_repr.end(); }

    template <typename F>
    void each(F&& f) { m_repr.each(FWD(f)); }
};

template <typename... Ws, typename... WOs>
class Query<With<Ws...>, Without<WOs...>, Group>
{
    using base_t = entt::group<entt::exclude_t<WOs...>, entt::get_t<>, Ws...>;
    base_t m_repr;

public:
    template <typename T>
    requires (!std::is_same_v<std::remove_cvref_t<T>, Query>)
        constexpr Query(T&& repr) noexcept : m_repr(FWD(repr)) {}

    auto begin() { return m_repr.begin(); }
    auto end() { return m_repr.end(); }

    template <typename F>
    void each(F&& f) { m_repr.each(FWD(f)); }
};