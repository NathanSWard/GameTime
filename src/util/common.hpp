#pragma once

#include <functional>
#include <type_traits>
#include <entt/entt.hpp>

#define UNUSED(x) static_cast<void>(x)
#define MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

using type_id_t = entt::id_type;

template <typename T>
constexpr auto type_id() noexcept -> type_id_t
{
    return entt::type_id<T>().hash();
}

struct in_place_t {};
inline constexpr in_place_t in_place{};

namespace hash {

    struct string_hash
    {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;

        size_t operator()(const char* str) const { return hash_type{}(str); }
        size_t operator()(std::string_view str) const { return hash_type{}(str); }
        size_t operator()(std::string const& str) const { return hash_type{}(str); }
    };

} // namespace hash