#pragma once

#include "core/game.hpp"
#include "core/ecs/commands.hpp"
#include "core/ecs/resource.hpp"
#include <chrono>
#include <tl/optional.hpp>

struct Time {
    using clock_t = std::chrono::high_resolution_clock;
    using duration_t = std::chrono::duration<float>;
    using time_point_t = std::chrono::time_point<clock_t>;

    tl::optional<time_point_t> last_update;
    duration_t delta;
    time_point_t startup;
};

void startup_time_system(Commands cmds)
{
    cmds.add_resource<Time>(Time {
            .last_update = {},
            .delta = {},
            .startup = Time::clock_t::now(),
        });
}

void update_time_system(Resource<Time> time)
{
    auto const now = Time::clock_t::now();

    if (time->last_update.has_value()) {
        time->delta = std::chrono::duration_cast<typename Time::duration_t>(now - time->last_update.value());
    }
    time->last_update = now;
}

struct TimePlugin
{
    void build(Game& game)
    {
        game.add_startup_system(startup_time_system);
        game.add_system(update_time_system);
    }
};