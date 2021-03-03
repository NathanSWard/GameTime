#pragma once

#include <core/game/events.hpp>
#include <core/game/game.hpp>
#include <core/input/input.hpp>
#include <core/input/keyboard.hpp>
#include <core/input/mouse.hpp>
#include <core/window/event.hpp>
#include <SDL2/SDL.h>

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
        .id = WindowId{ .id = e.windowID },
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

void sdl_handle_keyboard_event(
    Events<KeyboardInput>& keyboard_events,
    SDL_KeyboardEvent const& e)
{
    // TODO
}

void sdl_input_system(
    Resource<Events<GameExit>> game_exit_events,
    Resource<Events<MouseMotion>> mouse_motion_events,
    Resource<Events<CursorMoved>> cursor_moved_events,
    Resource<Events<MouseButtonInput>> mouse_button_events,
    Resource<Events<MouseWheel>> mouse_wheel_events,
    Resource<Events<KeyboardInput>> keyboard_events)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                sdl_handle_quit_event(*game_exit_events, e.quit);
                break;
            case SDL_MOUSEMOTION: 
                sdl_handle_mouse_motion_event(*mouse_motion_events, *cursor_moved_events, e.motion);
                break;
            case SDL_MOUSEBUTTONDOWN: 
                [[fallthrough]];
            case SDL_MOUSEBUTTONUP:
                sdl_handle_mouse_button_event(*mouse_button_events, e.button);
                break;
            case SDL_MOUSEWHEEL:
                sdl_handle_mouse_wheel_event(*mouse_wheel_events, e.wheel);
                break;
            case SDL_KEYDOWN: 
                [[fallthrough]];
            case SDL_KEYUP:
                sdl_handle_keyboard_event(*keyboard_events, e.key);
                break;
            default:
                break;
        }
    }
}