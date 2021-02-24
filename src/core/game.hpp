#pragma once

#include "debug/debug.hpp"
#include "ecs/resource.hpp"
#include "ecs/world.hpp"
#include "ecs/system_scheduler.hpp"

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

    template <Plugin P>
    auto add_plugin(P&& p) -> decltype(auto)
    {
        return FWD(p).build(*this);
    }

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

    template <typename R, typename... Args>
    auto add_resource(Args&&... args) -> Resource<R>
    {
        return m_resources.add_resource<R>(FWD(args)...);
    }

    void run()
    {
        auto const quit = m_resources.add_resource<Quit>();
        m_startup_scheduler.run_systems(m_resources, m_world);
        DEBUG_ASSERT(m_resources.contains_resource<Quit>());

        while (!((*quit).quit)) {
            m_scheduler.run_systems(m_resources, m_world);
            DEBUG_ASSERT(m_resources.contains_resource<Quit>());
        }
    }
};