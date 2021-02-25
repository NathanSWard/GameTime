#include <util/uuid.hpp>
#include <ut.hpp>
#include <unordered_map>

using namespace boost::ut;

void uuid_test()
{
    "[Uuid]"_test = [&] {
        auto const a = Uuid::create();
        auto const b = a;
        expect(a == b);

        auto const c = Uuid::create();
        expect(a != c);

        std::unordered_map<Uuid, int> _;
    };
}