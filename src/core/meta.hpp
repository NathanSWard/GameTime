#pragma once

#include <type_traits>
#include <tuple>

namespace meta {

    template <typename...>
    struct always_false : std::false_type {};

    template <typename...>
    struct args {};

    namespace internal {

        template <typename R, typename... Args>
        struct function_traits_defs
        {
            static constexpr std::size_t arg_count = sizeof...(Args);
            using result_t = R;
            using args_t = args<Args...>;

        private:
            template <std::size_t I>
            struct arg
            {
                using type = std::tuple_element_t<I, std::tuple<Args...>>;
            };

        public:
            template <std::size_t I>
            using arg_t = typename arg<I>::type;
        };

        template <typename T>
        struct function_traits_impl;

        template <typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(Args...)>
            : function_traits_defs<ReturnType, Args...> {};

        template <typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(*)(Args...)>
            : function_traits_defs<ReturnType, Args...> {};

        template <typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(*)(Args...) noexcept>
            : function_traits_defs<ReturnType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(ClassType::*)(Args...)>
            : function_traits_defs<ReturnType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(ClassType::*)(Args...) const>
            : function_traits_defs<ReturnType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(ClassType::*)(Args...) noexcept>
            : function_traits_defs<ReturnType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits_impl<ReturnType(ClassType::*)(Args...) const noexcept>
            : function_traits_defs<ReturnType, Args...> {};

    } // namespace internal
    
    // function_traits
    template <typename T, typename V = void>
    struct function_traits
        : internal::function_traits_impl<T> {};

    template <typename T>
    struct function_traits<T, decltype((void)&T::operator())>
        : internal::function_traits_impl<decltype(&T::operator())> {};

    // all
    template <template <typename...> typename Func, typename... Ts>
    inline constexpr bool all = (Func<Ts>::value && ...);

} // namespace meta