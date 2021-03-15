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
