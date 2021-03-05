#pragma once

#include <core/ecs/resource.hpp>
#include <core/game/events.hpp>
#include <core/window/event.hpp>

void exit_on_window_close_system(
    Resource<Events<GameExit>> game_exit_events,
    EventReader<WindowCloseRequest> window_close_requested_events)
{
    if (window_close_requested_events.iter().size() > 0) {
        game_exit_events->send(GameExit{});
    }
}