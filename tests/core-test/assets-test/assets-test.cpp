#include "ut.hpp"

#include <core/assets/assets.hpp>
#include <core/assets/handle.hpp>

using namespace boost::ut;

void assets_test()
{
    "[Assest]"_test = [] {
        auto as = Assets<int>();

        auto handle1 = as.add_asset(1);
        auto handle2 = as.add_asset(2);

        expect(handle1.id() != handle2.id());

        should("have same id") = [&] {
            auto handle1_copy = as.get_handle(handle1.id());
            expect(handle1.id() == handle1_copy.id());
        };

        should("contain assets") = [&] {
            expect(as.contains_asset(handle1));
            expect(as.contains_asset(handle2));
        };

        should("get handle's value") = [&] {
            auto value1 = as.get_asset(handle1);
            auto const value2a = as.cget_asset(handle2);
            auto const value2b = std::as_const(as).get_asset(handle2);

            static_assert(std::is_same_v<decltype(*value1), int&>);
            static_assert(std::is_same_v<decltype(*value2a), int const&>);
            static_assert(std::is_same_v<decltype(*value2b), int const&>);

            expect((value1.has_value() && value2a.has_value()) >> fatal);
            expect(*value1 == 1 && *value2a == 2 && *value2b == 2);
            expect(std::addressof(*value2a) == std::addressof(*value2b));
        };

        as.set_asset(handle1, 99);
        should("have updated asset") = [&] {
            auto value = as.get_asset(handle1);

            expect((value.has_value()) >> fatal);
            expect(*value == 99);
            
        };

        should("remove assets") = [&] {
            auto value1 = as.remove_asset(handle1);
            auto value2 = as.remove_asset(handle2);

            expect((value1.has_value()) >> fatal);
            expect((value2.has_value()) >> fatal);

            expect(*value1 == 99);
            expect(*value2 == 2);

            should("not have assets") = [&] {
                expect(!as.contains_asset(handle1));
                expect(!as.contains_asset(handle2));

                expect(!as.get_asset(handle1));
                expect(!as.get_asset(handle2));
            };
        };
    };
}