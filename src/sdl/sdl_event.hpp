#pragma once

#include <sdl/sdl.hpp>
#include <sdl/sdl_keyboard.hpp>
#include <core/game/events.hpp>
#include <core/game/game.hpp>
#include <core/input/input.hpp>
#include <core/input/keyboard.hpp>
#include <core/input/mouse.hpp>
#include <core/window/event.hpp>

namespace {

    void sdl_handle_quit_event(
        Events<GameExit>& exit_events,
        SDL_QuitEvent const&)
    {
        exit_events.send(GameExit{});
    }

    void sdl_handle_mouse_motion_event(
        Events<MouseMotion>& motion_events,
        Events<CursorMoved>& cursor_events,
        SDL_MouseMotionEvent const& e)
    {
        motion_events.send(MouseMotion{
            .delta = Vec2(static_cast<float>(e.xrel), static_cast<float>(e.yrel)),
            });

        cursor_events.send(CursorMoved{
            .id = WindowId{.id = e.windowID },
            .position = Vec2(static_cast<float>(e.x), static_cast<float>(e.y)),
            });
    }

    void sdl_handle_mouse_button_event(
        Events<MouseButtonInput>& button_events,
        SDL_MouseButtonEvent const& e)
    {
        auto const button = [&]() -> MouseButton {
            switch (e.button) {
            case SDL_BUTTON_LEFT:
                return MouseButton(MouseButton::Type::Left);
            case SDL_BUTTON_RIGHT:
                return MouseButton(MouseButton::Type::Right);
            case SDL_BUTTON_MIDDLE:
                return MouseButton(MouseButton::Type::Middle);
            case SDL_BUTTON_X1:
                return MouseButton(MouseButton::OtherButton{ .button = 1 });
            case SDL_BUTTON_X2:
                return MouseButton(MouseButton::OtherButton{ .button = 2 });
            default: // unreachable
                return MouseButton(MouseButton::OtherButton{ .button = 0 });
            }
        }();

        button_events.send(MouseButtonInput{
                .button = button,
                .state = e.state == SDL_PRESSED ? ElementState::Pressed : ElementState::Released,
                .clicks = e.clicks,
            });
    }

    void sdl_handle_mouse_wheel_event(
        Events<MouseWheel>& mouse_wheel_events,
        SDL_MouseWheelEvent const& e)
    {
        if (e.which == SDL_TOUCH_MOUSEID) {
            return;
        }

        auto const [x, y] = [&e]() -> std::tuple<int, int> {
            if (e.direction == SDL_MOUSEWHEEL_FLIPPED) {
                return std::make_tuple(e.x * -1, e.y * -1);
            }
            return std::make_tuple(e.x, e.y);
        }();

        mouse_wheel_events.send(MouseWheel{
            .x = static_cast<float>(x),
            .y = static_cast<float>(y),
            });
    }

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
                .path = std::string(e.file),
                });
        }
        SDL_free(e.file);
    }

} // namespace

void sdl_handle_event(Resources& resources)
{
    SDL_Event e{};
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: {
                auto events = resources.get_resource<Events<GameExit>>();
                DEBUG_ASSERT(events.has_value(), "Events<GameExit> does not exist.");
                sdl_handle_quit_event(**events, e.quit);
            } break;
            
            case SDL_MOUSEMOTION: {
                auto mouse_motion_events = resources.get_resource<Events<MouseMotion>>();
                DEBUG_ASSERT(mouse_motion_events.has_value(), "Events<MouseMotion> does not exist.");

                auto cursor_moved_events = resources.get_resource<Events<CursorMoved>>();
                DEBUG_ASSERT(cursor_moved_events.has_value(), "Events<CursorMoved> does not exist.");

                sdl_handle_mouse_motion_event(**mouse_motion_events, **cursor_moved_events, e.motion);
            } break;
            
            case SDL_MOUSEBUTTONDOWN: 
                [[fallthrough]];
            case SDL_MOUSEBUTTONUP: {
                auto events = resources.get_resource<Events<MouseButtonInput>>();
                DEBUG_ASSERT(events.has_value(), "Events<MouseButtonInput> does not exist.");
                sdl_handle_mouse_button_event(**events, e.button);
            } break;
           
            case SDL_MOUSEWHEEL: {
                auto events = resources.get_resource<Events<MouseWheel>>();
                DEBUG_ASSERT(events.has_value(), "Events<MouseWheel> does not exist.");
                sdl_handle_mouse_wheel_event(**events, e.wheel);
            } break;
            
            case SDL_KEYDOWN: 
                [[fallthrough]];
            case SDL_KEYUP: {
                auto events = resources.get_resource<Events<KeyboardInput>>();
                DEBUG_ASSERT(events.has_value(), "Events<KeyboardInput> does not exist.");
                sdl_handle_keyboard_event(**events, e.key);
            } break;
            
            case SDL_WINDOWEVENT:
                sdl_handle_window_event(e.window, resources);
                break;

            case SDL_DROPTEXT:
                [[fallthrough]];
            case SDL_DROPFILE: {
                auto events = resources.get_resource<Events<FileDragAndDrop>>();
                DEBUG_ASSERT(events.has_value(), "Events<FileDragAndDrop> does not exist.");
                sdl_handle_file_drag_and_drop(e.drop, **events);
            } break;
            
            default:
                break;
        }
    }
}