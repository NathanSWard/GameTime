#include "ut.hpp"
#include "core/ecs/resource.hpp"

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

    "[Resources]"_test = [] {
        Resources rm;
        
        should("add resource") = [&rm] {
            rm.set_resource<int>(42);
            expect(rm.contains_resource<int>());

            auto res = rm.get_resource<int>();
            auto cres = std::as_const(rm).get_resource<int>();
            auto cres2 = rm.get_resource<int const>();

            static_assert(std::is_same_v<std::remove_cvref_t<decltype(*res)>, Resource<int>>);
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(*cres)>, Resource<int const>>);
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(*cres2)>, Resource<int const>>);

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

        constexpr system_id_t id1 = 1;
        constexpr system_id_t id2 = 2;

        should("not conatins local resources") = [&] {
            expect(!rm.local().contains_local_resource<int>(id1));
            expect(rm.local().remove_local_resource<int>(id1) == nullptr);
        };

        should("add local resources") = [&rm] {
            rm.local().set_local_resource<int>(id1, 42);
            rm.local().set_local_resource<char>(id2, 'a');

            should("contains appropriate local resources") = [&] {
                expect(rm.local().contains_local_resource<int>(id1));
                expect(rm.local().contains_local_resource<char>(id2));

                expect(!rm.local().contains_local_resource<char>(id1));
                expect(!rm.local().contains_local_resource<int>(id2));
            };

            should("id1") = [&] {
                auto res = rm.local().get_local_resource<int>(id1);
                auto cres = std::as_const(rm).local().get_local_resource<int>(id1);
                auto cres2 = rm.local().get_local_resource<int const>(id1);

                static_assert(std::is_same_v<std::remove_cvref_t<decltype(*res)>, Local<int>>);
                static_assert(std::is_same_v<std::remove_cvref_t<decltype(*cres)>, Local<int const>>);
                static_assert(std::is_same_v<std::remove_cvref_t<decltype(*cres2)>, Local<int const>>);

                expect((res.has_value() && cres.has_value()) >> fatal);
                expect(std::addressof(**res) == std::addressof(**cres));
            };

            should("id2") = [&] {
                auto res = rm.local().get_local_resource<char>(id2);
                auto cres = std::as_const(rm).local().get_local_resource<char>(id2);
                auto cres2 = rm.local().get_local_resource<char const>(id2);

                expect((res.has_value() && cres.has_value()) >> fatal);
                expect(std::addressof(**res) == std::addressof(**cres));
            };
        };

        should("remove local resources") = [&rm] {
            expect(rm.local().remove_local_resource<int>(id1) != nullptr);
            expect(!rm.local().contains_local_resource<int>(id1));
            expect(!rm.local().get_local_resource<int>(id1).has_value());

            expect(rm.local().remove_local_resource<char>(id2) != nullptr);
            expect(!rm.local().contains_local_resource<char>(id2));
            expect(!rm.local().get_local_resource<char>(id2).has_value());

        };
    };
}