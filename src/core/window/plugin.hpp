#pragma once

#include <core/game/game.hpp>
#include <core/window/event.hpp>
#include <core/window/system.hpp>
#include <core/window/window.hpp>

struct WindowPlugin
{
    bool add_primary_window = true;
    bool exit_on_close = true;

    void build(GameBuilder& builder)
    {
        builder
            .add_event<WindowResized>()
            .add_event<CreateWindow>()
            .add_event<WindowCreated>()
            .add_event<WindowCloseRequest>()
            .add_event<CursorMoved>()
            .add_event<CursorEntered>()
            .add_event<CursorLeft>()
            .add_event<WindowFocused>()
            .add_event<FileDragAndDrop>()
            .add_event<WindowMoved>();

        if (add_primary_window) {
            auto& resources = builder.resources();
            auto window_settings = resources
                .get_resource<WindowSettings>()
                .map([](auto const& r) { return *r; })
                .value_or(WindowSettings{});

            auto create_window_event = resources.get_resource<Events<CreateWindow>>().value();
            create_window_event->send(CreateWindow{
                .settings = MOV(window_settings)
                });
        }

        if (exit_on_close) {
            builder.add_system(exit_on_window_close_system);
        }
    }
};