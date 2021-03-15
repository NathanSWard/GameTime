#pramga once

#include <debug/debug.hpp>
#include <fmt/format.h>
#include <tl/optional.hpp>
#include <util/common.hpp>
#include <util/containers/try.hpp>

template <typename T>
using optional = tl::optional<T>;

using nullopt_t = tl::nullopt_t;

inline constexpr nullopt_t nullopt = tl::nullopt;

template <typename T, typename... Args>
constexpr auto make_optional(Args&&... args)
{
    return tl::make_optional<T>(FWD(args)...);
}

template <typename Opt>
constexpr auto unwrap(Opt&& opt) -> decltype(auto)
{
    if (!opt.has_value()) [[unlikely]] {
        PANIC("{} did not contain a value.", type_name<std::remove_cvref_t<Opt>>());
    }
    return *FWD(opt);
}

template <typename Opt, typename... PanicArgs>
constexpr auto expect(Opt&& opt, std::string_view const fmt, PanicArgs&&... panic_args) -> decltype(auto)
{
    if (!opt.has_value()) [[unlikely]] {
        PANIC(fmt, FWD(panic_args)...);
    }
    return *FWD(opt);
}

template <typename T>
struct try_ops<optional<T>>
{
        static constexpr auto should_early_return(optional<T> const& opt) -> bool
        {
            return !opt.has_value();
        }

        static constexpr auto early_return(optional<T>&&)
        {
            return nullopt;
        }

        static constexpr auto extract_value(optional<T>&& o) -> decltype(auto)
        {
            return *MOV(o);
        }
};

template <typename T>
requires (Formattable<T>)
struct fmt::formatter<optional<T>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(optional<T> const& opt, Ctx& ctx) {
        if (opt.has_value()) {
            return format_to(ctx.out(), "optional<{}>::Some({})", type_name<T>(), *opt);
        }
        else {
            return format_to(ctx.out(), "optional<{}>::None", type_name<T>());
        }
    }
};




