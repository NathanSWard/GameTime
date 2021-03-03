#include <ut.hpp>
#include <util/ranges/chain.hpp>
#include <algorithm>

using namespace boost::ut;

void chain_test()
{
    auto vec1 = std::vector{ 1, 2, 3 };
    auto vec2 = std::vector{ 4, 5, 6 };

    auto chained = chain(vec1, vec2);

    auto first = chained.begin();
    expect(std::addressof(*first) == std::addressof(vec1[0]));

    int value = 1;
    for (auto const& v : chained) {
        expect(value == v);
        ++value;
    }
}