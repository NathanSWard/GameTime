#include <core/assets/handle.hpp>
#include <ut.hpp>
#include <unordered_map>

using namespace boost::ut;

void handle_test()
{
    "[Handle]"_test = [] {
        auto int_handle = Handle<int>::weak(HandleId::random<int>());
        auto char_handle = Handle<char>::weak(HandleId::random<char>());

        auto untyped_int_handle = int_handle.untyped();
        auto untyped_char_handle = char_handle.untyped();
            
        expect(int_handle.id() != char_handle.id());

        expect(int_handle.id() == untyped_int_handle.id());
        expect(char_handle.id() == untyped_char_handle.id());

        auto path_handle1 = Handle<int>::weak(HandleId::from_path("an-int-file"));
        auto path_handle2 = Handle<char>::weak(HandleId::from_path("a-char-file"));

        expect(path_handle1.id() != path_handle2.id());

        expect(int_handle.id() != path_handle1.id());
        expect(char_handle.id() != path_handle2.id());

        UNUSED(MOV(untyped_int_handle).typed<int>());
        UNUSED(MOV(untyped_char_handle).typed<char>());
    };
}