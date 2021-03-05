#pragma once

#include <SDL2/SDL.h>
#include <core/game/game.hpp>
#include <core/window/event.hpp>

/*
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
*/

void sdl_handle_window_event(
    SDL_WindowEvent const& e,
    Resources& resources)
{
    auto const id = WindowId{ .id = e.windowID };

    switch (e.event) {
        case SDL_WINDOWEVENT_RESIZED: {
            auto events = resources.get_resource<Events<WindowResized>>();
            DEBUG_ASSERT(events.has_value(), "Events<WindowResized> does not exist.");
            (**events).send(WindowResized{
                .id = id,
                .width = e.data1,
                .height = e.data2,
                });
        } break;

        case SDL_WINDOWEVENT_CLOSE: {
            auto events = resources.get_resource<Events<WindowCloseRequest>>();
            DEBUG_ASSERT(events.has_value(), "Events<WindowCloseRequest> does not exist.");
            (**events).send(WindowCloseRequest{ .id = id });
        } break;

        case SDL_WINDOWEVENT_ENTER: {
            auto events = resources.get_resource<Events<CursorEntered>>();
            DEBUG_ASSERT(events.has_value(), "Events<CursorEntered> does not exist.");
            (**events).send(CursorEntered{ .id = id });
        } break;

        case SDL_WINDOWEVENT_LEAVE: {
            auto events = resources.get_resource<Events<CursorLeft>>();
            DEBUG_ASSERT(events.has_value(), "Events<CursorLeft> does not exist.");
            (**events).send(CursorLeft{ .id = id });
        } break;

        case SDL_WINDOWEVENT_FOCUS_GAINED: {
            auto events = resources.get_resource<Events<WindowFocused>>();
            DEBUG_ASSERT(events.has_value(), "Events<WindowFocused> does not exist.");
            (**events).send(WindowFocused{ .id = id, .focused = true });
        } break;

        case SDL_WINDOWEVENT_FOCUS_LOST: {
            auto events = resources.get_resource<Events<WindowFocused>>();
            DEBUG_ASSERT(events.has_value(), "Events<WindowFocused> does not exist.");
            (**events).send(WindowFocused{ .id = id, .focused = false });
        } break;

        case SDL_WINDOWEVENT_MOVED: {
            auto events = resources.get_resource<Events<WindowMoved>>();
            DEBUG_ASSERT(events.has_value(), "Events<WindowMoved> does not exist.");
            (**events).send(WindowMoved{
                .id = id,
                .position = Vec2i{e.data1, e.data2}
                });
        } break;

        default: // TODO: possibly implement more later
            break;
    }
}

void sdl_handle_file_drag_and_drop(
    SDL_DropEvent const& e,
    Events<FileDragAndDrop>& events)
{
    if (e.type == SDL_DROPFILE) {
        events.send(FileDragAndDrop{
            .id = WindowId {.id = e.windowID},
            .path = std::filesystem::path(e.file),
            });
    }
    SDL_free(e.file);
}