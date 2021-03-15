#pragma once

#include <tl/expected.hpp>
#include <util/common.hpp>
#include <util/containers/try.hpp>

template <typename T, typename E>
using expected = tl::expected<T, E>;

template <typename E>
constexpr auto make_unexpected(E&& e)
{
    return tl::make_unexpected(FWD(e));
}

template <typename T, typename E>
struct try_ops<expected<T, E>>
{
    static constexpr auto should_early_return(expected<T, E> const& exp) -> bool
    {
        return !exp.has_value();
    }

    static constexpr auto early_return(expected<T, E>&& exp)
    {
        return make_unexpected(MOV(exp).error());
    }

    static constexpr auto extract_value(expected<T, E>&& exp) -> decltype(auto)
    {
        return *MOV(exp);
    }
};

template <typename T, typename E>
requires (Formattable<T> && Formattable<E>)
    struct fmt::formatter<expected<T, E>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(expected<T, E> const& exp, Ctx& ctx) {
        if (exp.has_value()) {
            return format_to(ctx.out(), "expected<{}, {}>::Ok({})", type_name<T>(), type_name<E>(), *exp);
        }
        else {
            return format_to(ctx.out(), "expected<{}, {}>::Err({})", type_name<T>(), type_name<E>(), exp.error());
        }
    }
};
