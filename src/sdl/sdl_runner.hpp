#pragma once

#include <core/game/game.hpp>
#include <core/render/render_context.hpp>
#include <core/window/event.hpp>
#include <core/window/window.hpp>

#include "sdl_event.hpp"

void handle_create_window_events(
    Resources& resources,
    ManualEventReader<InitializeWindow>& create_window_event_reader,
    ManualEventReader<ExitWindow>& exit_window_event_reader)
{
    auto windows = resources.get_resource<Windows>();
    DEBUG_ASSERT(windows.has_value(), "`Windows` resource does not exist");

    auto create_window_events = resources.get_resource<Events<InitializeWindow> const>();
    DEBUG_ASSERT(create_window_events.has_value(), "`Events<InitializeWindow>` resource does not exist");

    auto window_created_events = resources.get_resource<Events<WindowCreated>>();
    DEBUG_ASSERT(window_created_events.has_value(), "`Events<WindowCreated>` resource does not exist");

    auto create_window_event_iter = create_window_event_reader.iter(**create_window_events);
    for (auto const& create_window_event : create_window_event_iter) {
        auto window = Window::create(create_window_event.settings);
        if (!window) {
            // TODO: log the error
            continue;
        }

        auto const id = (**windows).add(*MOV(window));
        (**window_created_events).send(WindowCreated{ .id = id });
    }

    auto exit_window_events = resources.get_resource<Events<ExitWindow>>();
    DEBUG_ASSERT(exit_window_events.has_value(), "`Events<ExitWindow>` resource does not exist");

    auto exit_window_event_iter = exit_window_event_reader.iter(**exit_window_events);
    for (auto const& exit_window_event : exit_window_event_iter) {
        auto removed = (**windows).remove(exit_window_event.id);
        if (!removed) {
            // TODO: log the error
        }
    }
}

void handle_initialize_render_context_events(
    Resources& resources,
    ManualEventReader<InitializeRenderContext>& init_render_ctx_reader)
{
    // TODO
}

void sdl_runner(Game& game)
{
    auto create_window_event_reader = ManualEventReader<InitializeWindow>();
    auto create_render_context_reader = ManualEventReader<InitializeRenderContext>();
    auto exit_window_event_reader = ManualEventReader<ExitWindow>();
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
        handle_create_window_events(
            game.resources, 
            create_window_event_reader, 
            exit_window_event_reader);
        handle_initialize_render_context_events(
            game.resources,
            create_render_context_reader);
        game.update();
        
        if (check_for_app_exit()) {
            return;
        }
    }
}