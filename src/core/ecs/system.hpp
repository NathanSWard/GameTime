#pragma once

#include <entt/entt.hpp>
#include <core/game/events.hpp>
#include "commands.hpp"
#include "util/meta.hpp"
#include "util/common.hpp"
#include "query.hpp"
#include "resource.hpp"
#include "world.hpp"

class SystemSettings 
{
    SystemId m_id;
    bool m_should_run = true;

public:
    constexpr SystemSettings(SystemId const id, bool const should_run) noexcept
        : m_id(id)
        , m_should_run(should_run)
    {}

    constexpr SystemSettings(SystemSettings&&) noexcept = default;
    constexpr SystemSettings& operator=(SystemSettings&&) noexcept = default;

    constexpr auto id() const noexcept -> SystemId { return m_id; }
    constexpr auto should_run() const noexcept -> bool { return m_should_run; }
    constexpr void set_should_run(bool const b) noexcept { m_should_run = b; }
};

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
    constexpr void if_optional_check_value(T const&) {}

    template <typename O>
    requires (is_optional_v<O>)
    constexpr void if_optional_check_value(O const& opt)
    {
        // TODO: perhaps panic in release mode as well?
        DEBUG_ASSERT(opt.has_value(), "System was unable to find: '{}'.", type_name<typename O::value_type>());
        UNUSED(opt);
    }

    // checks a type to ensure its a valid system argument
    template <typename T>
    struct valid_system_arg : std::false_type {};

    template <typename... Qs>
    struct valid_system_arg<Query<Qs...>> : std::true_type {};

    template <typename R>
    struct valid_system_arg<Resource<R>> : std::true_type {};

    template <typename L>
    struct valid_system_arg<Local<L>> : std::true_type {};

    template <>
    struct valid_system_arg<Commands> : std::true_type {};

    template <typename T>
    struct valid_system_arg<EventReader<T>> : std::true_type {};

    // Depending on the argument type, returns the correct value from either the `Resources` or the `World`
    template <typename Arg>
    struct get_system_arg_impl;

    template <typename VG, typename... Ws, typename... WOs>
    struct get_system_arg_impl<Query<With<Ws...>, Without<WOs...>, VG>>
    {
        using query_t = Query<With<Ws...>, Without<WOs...>, VG>;
        auto operator()(SystemSettings const&, Resources&, World& world) const
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
        auto operator()(SystemSettings const&, Resources& res, World&) const
        {
            return res.get_resource<R>();
        }
    };

    template <typename L>
    struct get_system_arg_impl<Local<L>>
    {
        auto operator()(SystemSettings const& settings, Resources& res, World&) const
        {
            return res.local().get_local_resource<L>(settings.id());
        }
    };

    template <>
    struct get_system_arg_impl<SystemSettings>
    {
        constexpr auto operator()(SystemSettings& settings, Resources&, World&) const -> decltype(auto)
        {
            return std::ref(settings);
        }
    };

    template <>
    struct get_system_arg_impl<Commands>
    {
        constexpr auto operator()(SystemSettings const&, Resources& res, World& w) const -> Commands
        {
            return Commands{ res, w };
        }
    };

    template <typename T>
    struct get_system_arg_impl<EventReader<T>>
    {
        auto operator()(SystemSettings const& settings, Resources& res, World&) const -> EventReader<T>
        {
            auto events = res.get_resource<Events<T> const>();
            DEBUG_ASSERT(events.has_value(), "Events<{}> does not exist.", type_name<T>());

            using count_t = typename EventReader<T>::EventCount;
            auto const local = res.local().try_add_local_resource<count_t>(settings.id(), count_t{ 1 });
            return EventReader<T>(local, *MOV(events));
        }
    };

    template <typename... Args>
    auto get_system_args(SystemSettings& settings, Resources& res, World& world)
    {
        return std::make_tuple(get_system_arg_impl<Args>{}(settings, res, world)...);
    }

    // helper function that aggregate all the system arguments, check them, and then invoke the original function
    template <typename F, typename... Args>
    void type_erased_system_impl(SystemSettings& settings, void const* const data, Resources& res, World& world, meta::args<Args...>)
    {
        static_assert(meta::all<valid_system_arg, std::remove_cvref_t<Args>...>, "System arguments can only be a `Query<>` or `Resource<>`");
        
        auto const func = reinterpret_cast<F const*>(data);

        auto args = get_system_args<std::remove_cvref_t<Args>...>(settings, res, world);

        std::apply([](auto const&... xs) { (if_optional_check_value(xs), ...); }, args);

        std::apply([func](auto&&... xs) { (*func)(unwrap_optional(FWD(xs))...); }, MOV(args));
    }

    // type erased system function that reinterprets the function pointer to the original type
    template <typename F>
    void type_erased_system(SystemSettings& settings, void const* const data, Resources& resources, World& world)
    {
        using Func = std::remove_cvref_t<F>;
        using func_traits = meta::function_traits<Func>;
        static_assert(std::is_same_v<void, typename func_traits::result_t>, "Systems must return `void`");

        type_erased_system_impl<Func>(settings, data, resources, world, typename func_traits::args_t{});
    }

    using type_erased_system_t = void(*)(SystemSettings&, void const*, Resources&, World&);

} // namespace internal

class System 
{
    using run_func_t = internal::type_erased_system_t;

    run_func_t m_run_func;
    void const* m_data = nullptr;
    SystemSettings m_settings;

    System(run_func_t const run_func, void const* const data, SystemId const id) noexcept
        : m_run_func(run_func)
        , m_data(data)
        , m_settings(id, true)
    {}

public:
    constexpr System(System&&) noexcept = default;
    constexpr System& operator=(System&&) noexcept = default;

    template <typename F>
    requires (!std::is_rvalue_reference_v<F>)
    static auto create(F&& f) -> System
    {
        auto const id = SystemId::create<F>();
        auto const run_func = internal::type_erased_system<F>;
        return System(run_func, reinterpret_cast<void const*>(std::addressof(f)), id);
    }

    constexpr void run(Resources& resources, World& world)
    {
        m_run_func(m_settings, m_data, resources, world);
    }

    constexpr auto id() const noexcept -> SystemId { return m_settings.id(); }

    constexpr auto should_run() const noexcept -> bool { return m_settings.should_run(); }
};