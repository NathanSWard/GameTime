#pragma once

#include <core/game/game.hpp>
#include <core/render/render_context.hpp>
#include <core/window/event.hpp>
#include <core/window/window.hpp>

#include "sdl_event.hpp"

void sdl_runner(Game& game)
{
    auto game_exit_event_reader = ManualEventReader<GameExit>();

    auto check_for_app_exit = [&]() -> bool {
        auto events = game.resources.get_resource<Events<GameExit>>();
        if (!events) {
            return false;
        }

        if (game_exit_event_reader.iter(**events).size() > 0) {
            return true;
        }

        return false;
    };

    for (;;) {
        if (check_for_app_exit()) {
            return;
        }

        sdl_handle_event(game.resources);
        game.update();
        
        if (check_for_app_exit()) {
            return;
        }
    }
}