#pragma once

#include <debug/debug.hpp>
#include "ecs/resource.hpp"
#include "ecs/world.hpp"
#include "ecs/system_scheduler.hpp"
#include <core/assets/asset_server.hpp>

class Game;

struct Quit {
    bool quit = false;
};

template <typename P>
concept Plugin = requires(P&& plugin, Game& game)
{
    { FWD(plugin).build(game) };
};

class Game {
    World m_world;
    Resources m_resources;
    Scheduler m_startup_scheduler;
    Scheduler m_scheduler;

public:
    // resources
    auto resources() const noexcept -> Resources const&
    {
        return m_resources;
    }

    auto resources() noexcept -> Resources&
    {
        return m_resources;
    }

    template <typename R, typename... Args>
    auto try_add_resource(Args&&... args) -> Resource<R>
    {
        return m_resources.try_add_resource<R>(FWD(args)...);
    }

    template <typename R, typename... Args>
    auto set_resource(Args&&... args) -> Resource<R>
    {
        return m_resources.set_resource<R>(FWD(args)...);
    }

    // plugins
    template <Plugin P>
    auto add_plugin(P&& p) -> decltype(auto)
    {
        return FWD(p).build(*this);
    }

    // systems
    template <typename S>
    auto add_system(S&& s) -> system_id_t
    {
        return m_scheduler.add_system(FWD(s));
    }

    template <typename S>
    auto add_startup_system(S&& s) -> system_id_t
    {
        return m_startup_scheduler.add_system(FWD(s));
    }

    // components
    template <typename... Cs>
    auto prepare_components() -> Game&
    {
        (m_world.prepare<Cs>(), ...);
        return *this;
    }

    // assets
    template <typename T>
    auto add_asset() -> Game&
    {
        DEBUG_ASSERT(resources().contains_resource<AssetServer>());
        auto server = resources().get_resource<AssetServer>().value();
        add_resource<Assets<T>>(server->register_asset_type<T>());
        add_system(update_assets_system<T>);
        prepare_components<Handle<T>>();
        return *this;
    }

    template <typename T, typename... Args>
    auto add_asset_loader(Args&&... args) -> Game&
    {
        DEBUG_ASSERT(resources().contains_resource<AssetServer>());
        auto server = resources().get_resource<AssetServer>().value();
        server->add_asset_loader<T>(FWD(args)...);
        return *this;
    }

    // run
    void run()
    {
        auto const quit = m_resources.set_resource<Quit>();
        m_startup_scheduler.run_systems(m_resources, m_world);
        DEBUG_ASSERT(m_resources.contains_resource<Quit>());

        while (!((*quit).quit)) {
            m_scheduler.run_systems(m_resources, m_world);
            DEBUG_ASSERT(m_resources.contains_resource<Quit>());
        }
    }
};