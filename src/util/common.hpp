#pragma once

#include <functional>
#include <fmt/format.h>
#include <type_traits>
#include <entt/entt.hpp>
#include <util/meta.hpp>

#define UNUSED(x) static_cast<void>(x)
#define MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

template <typename T>
constexpr auto type_name() noexcept -> std::string_view
{
    return entt::type_name<T>::value();
}

class type_id_t
{
    std::size_t m_hash = 0;
    std::string_view m_name = "Invalide Type Name";

public:
    constexpr type_id_t() noexcept = default;

    constexpr type_id_t(std::size_t const hash, std::string_view const name) noexcept
        : m_hash(hash)
        , m_name(name)
    {}

    constexpr type_id_t(type_id_t&&) noexcept = default;
    constexpr type_id_t(type_id_t const&) noexcept = default;
    constexpr type_id_t& operator=(type_id_t&&) noexcept = default;
    constexpr type_id_t& operator=(type_id_t const&) noexcept = default;

    constexpr auto operator==(type_id_t const& other) const noexcept -> bool
    {
        return m_hash == other.m_hash && m_name.data() == other.m_name.data();
    }

    constexpr auto operator!=(type_id_t const& other) const noexcept -> bool
    {
        return !(*this == other);
    }

    constexpr auto hash() const noexcept -> std::size_t { return m_hash; }
    constexpr auto name() const noexcept -> std::string_view { return m_name; }
};

template <>
struct std::hash<type_id_t>
{
    auto operator()(type_id_t const id) const noexcept
    {
        return id.hash();
    }
};

template <typename T>
constexpr auto type_id() noexcept -> type_id_t
{
    return type_id_t{ entt::type_hash<T>::value(), type_name<T>() };
}

struct in_place_t {};
inline constexpr in_place_t in_place{};

template <typename T>
struct in_place_type_t {};

template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};

namespace hash {

    struct string_hash
    {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = std::true_type;

        std::size_t operator()(char const* const str) const noexcept { return hash_type{}(str); }
        std::size_t operator()(std::string_view const str) const noexcept { return hash_type{}(str); }
        std::size_t operator()(std::string const& str) const noexcept { return hash_type{}(str); }
    };

    struct string_equal
    {
        using is_transparent = std::true_type;

        constexpr auto operator()(std::string_view const lhs, std::string_view const rhs) const noexcept -> bool
        {
            return lhs == rhs;
        }
    };

} // namespace hash

template <typename T>
concept Formattable = requires(T const& t)
{
    { fmt::format("{}", t) };
};

// bit_cast
#if __cpp_lib_bit_cast >= 201896L
    #include <bit>

#define CONSTEXPR_BIT_CAST constexpr

    template <typename To, typename From>
    auto bit_cast(const From& src) noexcept -> To
    {
        return std::bit_cast<To>(src);
    }
#else

#define CONSTEXPR_BIT_CAST

    template <typename To, typename From>
    requires (sizeof(To) == sizeof(From) &&
              std::is_trivially_copyable_v<From> &&
              std::is_trivially_copyable_v<To>)
    // constexpr support needs compiler magic
    auto bit_cast(const From& src) noexcept -> To
    {
        static_assert(std::is_trivially_constructible_v<To>,
                      "This implementation additionally requires destination type to be trivially constructible");

        To dst;
        std::memcpy(&dst, &src, sizeof(To));
        return dst;
    }
#endif