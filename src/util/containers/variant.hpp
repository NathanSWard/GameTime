#pragma once

#include <boost/variant2/variant.hpp>
#include <boost/preprocesor/repeat.hpp>
#include <util/common.hpp>

using variant = boost::variant2::variant;

// https://www.reddit.com/r/cpp/comments/kst2pu/with_stdvariant_you_choose_either_performance_or/giilcxv?utm_source=share&utm_medium=web2x&context=3

template <std::size_t I = 0>
BOOST_FORCEINLINE constexpr auto visit(auto&& f, auto&& v) -> decltype(auto)
{
    constexpr auto vs = boost::variant2::variant_size_v<std::remove_cvref_t<decltype(v)>>;

#define VISIT_CASE_COUNT 32
#define VISIT_CASE(Z, N, D) \
        case I + N: { \
            if constexpr(I + N < vs) { \
                return FWD(f)(*boost::variant2::get_if<I + N>(&FWD(v))); \
            } \
            else { \
                BOOST_UNREACHABLE_RETURN(FWD(f)(*boost::variant2::get_if<0>(&FWD(v)))); \
            } \
        }

    switch(v.index()) {
        BOOST_PP_REPEAT(VISIT_CASE_COUNT, VISIT_CASE, _)
    }

    constexpr auto next_idx = std::min(I + VISIT_CASE_COUNT, vs);

    if constexpr(next_idx + 0 < vs)
    {
        return visit1<next_idx>(FWD(f), FWD(v));
    }

    BOOST_UNREACHABLE_RETURN(FWD(f)(*boost::variant2::get_if<0>(&FWD(v))));

#undef VISIT_CASE_COUNT
#undef VISIT_CASE
}
