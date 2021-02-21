#include <cassert>

#include <string>
#include <util/type_map.hpp>

void test_type_map()
{
    TypeMap tm;
    assert(tm.size() == 0);
    assert(tm.empty());


    auto& i = tm.insert<int>(42);
    assert(i == 42);
    assert(tm.size() == 1);
    assert(!tm.empty());

    auto check_int = [&tm]() {
        auto found = tm.get<int>();
        assert(found.has_value());
        assert(found.value() == 42);
    };
    check_int();


    auto& s = tm.insert<std::string>("test");
    assert(s == "test");
    assert(tm.size() == 2);

    auto found_string = tm.get<std::string>();
    assert(found_string.has_value());
    assert(found_string.value() == "test");

    check_int();

    // remove something that is NOT in the map
    {
        auto const ptr = tm.remove<float>();
        assert(ptr == nullptr);
        assert(tm.size() == 2);
    }

    // remove something that IS in the map
    {
        auto const str_ptr = tm.remove<std::string>();
        assert(str_ptr != nullptr);
        assert(*str_ptr == "test");
        assert(tm.size() == 1);
    }

    {
        auto const int_ptr = tm.remove<int>();
        assert(int_ptr != nullptr);
        assert(*int_ptr == 42);
        assert(tm.size() == 0);
        assert(tm.empty());
    }
}

int main(int, char**)
{
    test_type_map();
    return 0;
}

#ifdef _WIN32
#include <Windows.h>
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif

