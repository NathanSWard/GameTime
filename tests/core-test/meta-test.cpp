#include <core/meta.hpp>

void func(int&, float) {}

using func_traits = meta::function_traits<decltype(func)>;
static_assert(std::is_same_v<func_traits::result_t, void>);
static_assert(func_traits::arg_count == 2);
static_assert(std::is_same_v<func_traits::arg_t<0>, int&>);
static_assert(std::is_same_v<func_traits::arg_t<1>, float>);

void meta_test()
{
    auto lambda = [](char) -> int { return 0; };
    using l_func_traits = meta::function_traits<decltype(lambda)>;
    static_assert(std::is_same_v<l_func_traits::result_t, int>);
    static_assert(l_func_traits::arg_count == 1);
    static_assert(std::is_same_v<l_func_traits::arg_t<0>, char>);
}