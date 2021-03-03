#pragma once

#include <util/common.hpp>
#include <fmt/format.h>
#include <string_view>
#include <sstream>
#include <thread>
#include <tuple>

#if __cpp_consteval >= 201811L
    #define CONSTEVAL consteval
#else 
    #define CONSTEVAL constexpr
#endif

#if __cpp_lib_source_location >= 201907L
    #include <source_loaction>
    using source_location = std::source_location;
#else
    struct source_location {
    private:
        std::uint_least32_t m_line{};
        std::uint_least32_t m_column{};
        char const* m_file = "";
        char const* m_function = "";
    public:
        [[nodiscard]] static CONSTEVAL source_location current(
            std::uint_least32_t const line = __builtin_LINE(),
            std::uint_least32_t const column = __builtin_COLUMN(), 
            char const* const file = __builtin_FILE(),
            char const* const function = __builtin_FUNCTION()) noexcept 
        {
            source_location loc;
            loc.m_line = line;
            loc.m_column = column;
            loc.m_file = file;
            loc.m_function = function;
            return loc;
        }

        [[nodiscard]] constexpr source_location() noexcept = default;

        [[nodiscard]] constexpr std::uint_least32_t line() const noexcept { return m_line; }
        [[nodiscard]] constexpr std::uint_least32_t column() const noexcept { return m_column; }
        [[nodiscard]] constexpr const char* file_name() const noexcept { return m_file; }
        [[nodiscard]] constexpr const char* function_name() const noexcept { return m_function; }
    };
#endif // __cpp_lib_source_location >= 201907L

template <typename... Args>
auto args(Args&&... args)
{
    return std::forward_as_tuple(FWD(args)...);
}

template <typename Tpl = std::tuple<>>
[[noreturn]] void panic(
    std::string_view const fmt = "explicit panic", 
    Tpl args = std::tuple<>{}, // use args() to pass the arguments
    source_location const& loc = source_location::current())
{
    auto const message = std::apply([fmt](auto&&... args) {
            return fmt::format(fmt, FWD(args)...);
        }, MOV(args));

    std::stringstream ss;
    ss << std::this_thread::get_id();

    fmt::print(stderr,
        "thread '{}' panicked at '{}', {}:{}:{}, '{}'\n",
        ss.str(),
        message,
        loc.file_name(),
        loc.line(),
        loc.column(),
        loc.function_name());
    std::abort();
}

#ifndef NDEBUG
    #define IS_DEBUG
#endif

// DEBUG_ASSERT
#ifdef IS_DEBUG
    #include <cassert>
    #define DEBUG_ASSERT(x, ...) \
        if (!static_cast<bool>(x)) { \
            panic(__VA_ARGS__); \
        }
    #define DEBUG_ASSERT_EQ(x, y, ...) DEBUG_ASSERT((x == y), __VA_ARGS__)
    #define DEBUG_ASSERT_NEQ(x, y, ...) DEBUG_ASSERT((x != y), __VA_ARGS__)
#else
    #define DEBUG_ASSERT(...)
    #define DEBUG_ASSERT_EQ(...)
    #define DEBUG_ASSERT_NEQ(...)
#endif