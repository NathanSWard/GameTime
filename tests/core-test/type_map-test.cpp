#include "../ut/ut.hpp"
#include "core/type_map.hpp"

#include <string_view>

using namespace boost::ut;

class InvokeDestructor
{
    int* i = nullptr;

public:
    constexpr InvokeDestructor(int& i) noexcept : i(&i) {}

    ~InvokeDestructor()
    {
       ++(*i);
    }
};

void type_map_test()
{
    "[TypeMap]"_test = [] {
        TypeMap tm;
        
        should("be empty") = [&tm] {
            expect(tm.empty());
            expect(tm.size() == 0);
            expect(!tm.contains<int>());
        };

        expect(tm.insert<int>(42) == 42);
        should("have one element") = [&tm] {
            expect(!tm.empty());
            expect(tm.size() == 1);
            expect(tm.contains<int>());
        };

        expect(tm.insert<std::string_view>("hello") == "hello");
        should("have two elements") = [&tm] {
            expect(!tm.empty());
            expect(tm.size() == 2);
            expect(tm.contains<int>() && tm.contains<std::string_view>());
        };

        should("remove int") = [&tm] {
            auto const ptr = tm.remove<int>();
            expect((ptr != nullptr) >> fatal);
            expect(*ptr == 42);
            expect(!tm.empty() && tm.size() == 1);
            expect(!tm.contains<int>());
        };

        should("remove string_view") = [&tm] {
            auto const ptr = tm.remove<std::string_view>();
            expect((ptr != nullptr) >> fatal);
            expect(*ptr == "hello");
            expect(tm.empty() && tm.size() == 0);
            expect(!tm.contains<std::string_view>());
        };

        should("invoke destructor") = [] {
            int count = 0;
            {
                TypeMap tm;
                tm.insert<InvokeDestructor>(count);
            }
            expect(count == 1);
        };
    };
}