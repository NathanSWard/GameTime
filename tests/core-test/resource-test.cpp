#include "../ut/ut.hpp"
#include "core/resource.hpp"

using namespace boost::ut;

void resource_test()
{
    "[Resource]"_test = [] {

        should("non-const resource") = [] {
            int i = 0;
            auto res = make_resource<int>(i);
            expect(std::addressof(*res) == std::addressof(i));
            expect(*res == i);

            ++i;
            expect(*res == 1);
            expect(*res == i);

            ++(*res);
            expect(i == 2);
            expect(*res == i);
        };

        should("const resource") = [] {
            {
                int i = 0;
                auto res = make_const_resource<int>(i);
                static_assert(std::is_same_v<std::remove_cvref_t<decltype(res)>, Resource<int const>>);
                expect(std::addressof(*res) == std::addressof(i));
                expect(*res == i);

                ++i;
                expect(*res == 1);
                expect(*res == i);
            }

            {
                int const i = 0;
                auto res = make_const_resource<int>(i);
                static_assert(std::is_same_v<std::remove_cvref_t<decltype(res)>, Resource<int const>>);
                expect(std::addressof(*res) == std::addressof(i));
                expect(*res == i);
            }
        };
    };

    "[ResourceManager]"_test = [] {
        ResourceManager rm;
        
        should("add resource") = [&rm] {
            rm.add_resource<int>(42);
            expect(rm.contains_resource<int>());

            auto res = rm.get_resource<int>();
            auto cres = std::as_const(rm).get_resource<int>();

            static_assert(std::is_same_v<std::remove_cvref_t<decltype(*res)>, Resource<int>>);
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(*cres)>, Resource<int const>>);

            expect((res.has_value() && cres.has_value()) >> fatal);
            expect(std::addressof(**res) == std::addressof(**cres));
            
            ++(**res);
            expect(**res == **cres);
        };

        should("remove resource") = [&rm] {
            expect(rm.remove_resource<int>() != nullptr);
            expect(!rm.contains_resource<int>());
            expect(!rm.get_resource<int>().has_value());
        };
    };
}