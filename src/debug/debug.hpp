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
auto panic_args(Args&&... args)
{
    return std::forward_as_tuple(FWD(args)...);
}

template <typename Tpl = std::tuple<>>
[[noreturn]] void panic(
    std::string_view const fmt = "explicit panic", 
    Tpl args = std::tuple<>{}, // use panic_args() to pass the arguments
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

#define PANIC(fmt, ...) panic(fmt, panic_args(__VA_ARGS__))

#ifndef NDEBUG
    #define IS_DEBUG
#endif

// DEBUG_ASSERT
#ifdef IS_DEBUG
    #define DEBUG_ASSERT(x, fmt, ...) \
        if (!static_cast<bool>(x)) [[unlikely]] { \
            PANIC(fmt, __VA_ARGS__); \
        }
#else
    #define DEBUG_ASSERT(...)
#endif


// LOGGING

#include <spdlog/spdlog.h>

struct LogLevel
{
    enum level_enum {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
    };

    static constexpr char const* level_name[] = { "Trace", "Debug", "Info", "Warn", "Error", "Critical" };

    constexpr static auto to_string(level_enum const level) noexcept -> std::string_view
    {
        switch (level) {
            case LogLevel::Trace: [[fallthrough]];
            case LogLevel::Debug: [[fallthrough]];
            case LogLevel::Info: [[fallthrough]];
            case LogLevel::Warn: [[fallthrough]];
            case LogLevel::Error: [[fallthrough]];
            case LogLevel::Critical: 
                return level_name[static_cast<std::size_t>(level)];
            default: 
                return "unknown log level";
        };
    }
};

inline void set_log_level(LogLevel::level_enum const level)
{
    auto const spd_log_level = [level] {
        switch (level) {
        case LogLevel::Trace: return spdlog::level::trace;
        case LogLevel::Debug: return spdlog::level::debug;
        case LogLevel::Info: return spdlog::level::info;
        case LogLevel::Warn: return spdlog::level::warn;
        case LogLevel::Error: return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        };
    }();

    spdlog::set_level(spd_log_level);
}

#define LOG_CRITICAL(fmt, ...) spdlog::critical(fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) spdlog::error(fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) spdlog::warn(fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) spdlog::info(fmt, __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) spdlog::debug(fmt, __VA_ARGS__)
#define LOG_TRACE(fmt, ...) spdlog::trace(fmt, __VA_ARGS__)