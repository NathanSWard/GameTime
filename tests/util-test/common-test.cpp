#include <util/common.hpp>
#include <ut.hpp>

using namespace boost::ut;

void common_test()
{
    "[util::fs::get_extension]"_test = [] {
        expect(fs::get_extension("hello.txt") == ".txt");
        expect(fs::get_extension("hello") == "");
        expect(fs::get_extension("") == "");
        expect(fs::get_extension("hello.") == ".");
    };
}