#pragma once

#include <core/game/game.hpp>
#include <core/window/event.hpp>
#include <core/window/system.hpp>
#include <core/window/window.hpp>
#include <debug/debug.hpp>

struct WindowPlugin
{
    bool exit_on_close = true;

    void build(GameBuilder& builder)
    {
        builder
            .add_event<WindowResized>()
            .add_event<WindowCloseRequest>()
            .add_event<ExitWindow>()
            .add_event<CursorMoved>()
            .add_event<CursorEntered>()
            .add_event<CursorLeft>()
            .add_event<WindowFocused>()
            .add_event<FileDragAndDrop>()
            .add_event<WindowMoved>();

        auto& resources = builder.resources();
        auto const window_settings = resources
            .get_resource<WindowSettings>()
            .map([](auto const& r) { return *r; })
            .value_or(WindowSettings{});

        auto window = Window::create(window_settings);
        if (!window) {
            panic("Window failed to initialize! Error: {}", panic_args(window.error().msg));
            return;
        }

        builder.set_resource<Window>(*MOV(window));

        if (exit_on_close) {
            builder.add_system(exit_on_window_close_system);
        }
    }
};