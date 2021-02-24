#pragma once

#include <functional>
#include <type_traits>

#define UNUSED(x) static_cast<void>(x)
#define MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

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

namespace fs { // filesystem 

    constexpr auto get_extension(std::string_view const path) noexcept -> std::string_view
    {
        if (auto const pos = path.find_last_of('.'); pos != std::string_view::npos) {
            return path.substr(pos);
        }
        return "";
    }

} // namespace filesystem