#pragma once

#include <entt/entt.hpp>
#include "commands.hpp"
#include "util/meta.hpp"
#include "util/common.hpp"
#include "query.hpp"
#include "resource.hpp"
#include "world.hpp"

namespace internal {

    // check if a type is a `tl::optional`
    template <typename T>
    struct is_optional_impl : std::false_type {};

    template <typename T>
    struct is_optional_impl<tl::optional<T>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_optional_v = is_optional_impl<std::remove_cvref_t<T>>::value;

    // if a value is an option, return its contained value, else return the original value.
    template <typename T>
    requires (!is_optional_v<T>)
    constexpr auto unwrap_optional(T&& t) -> decltype(auto)
    {
        return FWD(t);
    }

    template <typename O>
    requires (is_optional_v<O>)
    constexpr auto unwrap_optional(O&& opt)
    {
        return *FWD(opt);
    }

    // if a value is an optional check `has_value()`, else return true
    template <typename T>
    requires (!is_optional_v<T>)
    constexpr auto if_optional_check_value(T const&) -> bool
    {
        return true;
    }

    template <typename O>
    requires (is_optional_v<O>)
    constexpr auto if_optional_check_value(O const& opt) -> bool
    {
        return opt.has_value();
    }

    // checks a type to ensure its a valid system argument
    template <typename T>
    struct valid_system_arg : std::false_type {};

    template <typename... Qs>
    struct valid_system_arg<Query<Qs...>> : std::true_type {};

    template <typename R>
    struct valid_system_arg<Resource<R>> : std::true_type {};

    template <>
    struct valid_system_arg<Commands> : std::true_type {};
    

    // Depending on the argument type, returns the correct value from either the `Resources` or the `World`
    template <typename Arg>
    struct get_system_arg_impl;

    template <typename VG, typename... Ws, typename... WOs>
    struct get_system_arg_impl<Query<With<Ws...>, Without<WOs...>, VG>>
    {
        using query_t = Query<With<Ws...>, Without<WOs...>, VG>;
        auto operator()(Resources&, World& world) -> decltype(auto)
        {
            if constexpr (std::is_same_v<VG, View>) {
                return query_t{ world.view<Ws...>(entt::exclude<WOs...>) };
            }
            else if constexpr (std::is_same_v<VG, Group>) {
                return query_t{ world.group<Ws...>(entt::exclude<WOs...>) };
            }
            else {
                static_assert(meta::always_false<VG>::value, "Final Query<> param must be either a `View` or a `Group`");
            }
        }
    };

    template <typename R>
    struct get_system_arg_impl<Resource<R>>
    {
        auto operator()(Resources& res, World&) -> decltype(auto)
        {
            return res.get_resource<R>();
        }
    };

    template <>
    struct get_system_arg_impl<Commands>
    {
        constexpr auto operator()(Resources& res, World& w) -> Commands
        {
            return Commands{ res, w };
        }
    };

    template <typename... Args>
    auto get_system_args(Resources& res, World& world)
    {
        return std::make_tuple(get_system_arg_impl<Args>{}(res, world)...);
    }

    // helper function that aggregate all the system arguments, check them, and then invoke the original function
    template <typename F, typename... Args>
    void type_erased_system_impl(void* const data, Resources& res, World& world, meta::args<Args...>)
    {
        static_assert(meta::all<valid_system_arg, std::remove_cvref_t<Args>...>, "System arguments can only be a `Query<>` or `Resource<>`");
        
        auto const func = static_cast<F*>(data);

        auto args = get_system_args<std::remove_cvref_t<Args>...>(res, world);

        bool const has_all_resouces = std::apply([](auto const&... xs) {
            return (if_optional_check_value(xs) && ...);
        }, args);

        if (has_all_resouces) {
            std::apply([func](auto&&... xs) { (*func)(unwrap_optional(FWD(xs))...); }, MOV(args));
        }
    }

    // type erased system function that reinterprets the function pointer to the original type
    template <typename F>
    void type_erased_system(void* const data, Resources& resources, World& world)
    {
        using Func = std::remove_cvref_t<F>;
        using func_traits = meta::function_traits<Func>;
        static_assert(std::is_same_v<void, typename func_traits::result_t>, "Systems must return `void`");

        type_erased_system_impl<Func>(data, resources, world, typename func_traits::args_t{});
    }

    using type_erased_system_t = void(*)(void*, Resources&, World&);

} // namespace internal

class System 
{
    using run_func_t = internal::type_erased_system_t;

    run_func_t m_run_func;
    void* m_data = nullptr;

    constexpr System(run_func_t const run_func, void* const data) noexcept 
        : m_run_func(run_func)
        , m_data(data)
    {
    }

public:
    constexpr System(System&&) noexcept = default;
    constexpr System(System const&) noexcept = default;
    constexpr System& operator=(System&&) noexcept = default;
    constexpr System& operator=(System const&) noexcept = default;

    template <typename F>
    requires (!std::is_rvalue_reference_v<F>)
    static constexpr auto create(F&& f) -> System
    {
        auto const run_func = internal::type_erased_system<F>;
        return System(run_func, std::addressof(f));
    }

    constexpr void run(Resources& resources, World& world)
    {
        m_run_func(m_data, resources, world);
    }
};