#include "../ut/ut.hpp"
#include "util/rng.hpp"

#include <cmath>

using namespace boost::ut;

void rng_test()
{
    "[Random Number - No Limit]"_test = [] {
        auto const i = util::uniform_rand<int>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(i)>, int>);

        auto const f = util::uniform_rand<float>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(f)>, float>);
    };

    "[Random Number - Min/Max]"_test = [] {
        auto const i = util::uniform_rand<int>(0, 10);
        expect(i >= 0 && i <= 10);

        auto const f = util::uniform_rand<float>(0.f, 10.f);
        expect(std::isgreaterequal(f, 0.f) && std::islessequal(f, 10.f));
    };

    "[Fast Random Number]"_test = [] {
        auto const i = util::fast_rand<int>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(i)>, int>);

        auto const f = util::fast_rand<float>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(f)>, float>);

        auto const d = util::fast_rand<double>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(d)>, double>);

        auto const ld = util::fast_rand<long double>();
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(ld)>, long double>);
    };
}