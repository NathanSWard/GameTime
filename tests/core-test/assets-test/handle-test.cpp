#include <core/assets/handle.hpp>
#include <core/assets/assets.hpp>
#include <ut.hpp>

using namespace boost::ut;

void handle_test()
{
    "[Handle]"_test = [] {
        auto int_assets = Assets<int>{};
        auto char_assets = Assets<char>{};

        auto int_handle = int_assets.add_asset(0);
        auto char_handle = char_assets.add_asset('a');

        auto untyped_int_handle = int_handle.untyped();
        auto untyped_char_handle = char_handle.untyped();

        expect(int_handle == untyped_int_handle);
        expect(char_handle == untyped_char_handle);

        expect(untyped_int_handle.typed<int>().has_value());
        expect(!untyped_int_handle.typed<char>().has_value());
    };
}