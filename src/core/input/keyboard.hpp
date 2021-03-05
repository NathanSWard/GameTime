#pragma once

#include <core/ecs/resource.hpp>
#include <core/game/events.hpp>
#include <core/input/input.hpp>
#include <cstdint>
#include <fmt/format.h>
#include <functional>
#include <tl/optional.hpp>

enum class KeyCode : std::uint32_t
{
    Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9, Key0,

    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    Esc,

    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,

    PrintScreen,
    ScrollLock,
    PauseBreak,

    Insert,
    Home,
    Delete,
    End,
    PageUp,
    PageDown,

    Up, Down, Left, Right,

    Backspace, Enter, Space,

    Application, Sleep,

    NumlockClear,
    NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
    NumPadDivide, NumPadMultiply, NumPadSubtract, NumPadPlus, NumPadEnter, NumPadDecimal, NumPadComma,

    Comma, Decimal, FwdSlash, BackSlash, SingleQuote, SemiColon, Equal, Subtract, BackQuote, LBracket, RBracket,

    Exclaim, At, Hash, Dollar, Percent, Caret, Ampersand, Asterisk, LParen, RParen, Underscore,
    Plus, DblQuote, Colon, Question, Greater, Less,

    Tab, CapsLock, LShift, RShift, LCtrl, RCtrl, RAlt, LAlt, RGui, LGui,

    Mute, Stop, PrevTrack, PlayPause, NextTrack, VolumeUp, VolumeDown,

    UNKNOWN,
};

template <>
struct std::hash<KeyCode>
{
    auto operator()(KeyCode const& keycode) const noexcept -> std::size_t
    {
        return static_cast<std::size_t>(static_cast<std::uint32_t>(keycode));
    }
};

struct KeyboardInput
{
    std::uint32_t scan_code = 0;
    KeyCode key_code = KeyCode::UNKNOWN;
    ElementState state;
};

void keyboard_input_system(
    Resource<Input<KeyCode>> keyboard_input,
    EventReader<KeyboardInput> keyboard_input_event)
{
    keyboard_input->update();
    auto events = keyboard_input_event.iter();
    for (auto const& e : events) {
        if (e.key_code != KeyCode::UNKNOWN) {
            switch (e.state.state) {
                case ElementState::Pressed: 
                    keyboard_input->press(e.key_code);
                    break;
                case ElementState::Released: 
                    keyboard_input->release(e.key_code);
                    break;
                default: // unreachable
                    break;
            }
        }
    }
}

constexpr char const* KEYCODE_STR_MAP[] = {
    "Key1", "Key2", "Key3", "Key4", "Key5", "Key6", "Key7", "Key8", "Key9", "Key0",

    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",

    "Esc",

    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24",

    "PrintScreen",
    "ScrollLock",
    "PauseBreak",

    "Insert",
    "Home",
    "Delete",
    "End",
    "PageUp",
    "PageDown",

    "Up", "Down", "Left", "Right",

    "Backspace", "Enter", "Space",

    "Application", "Sleep",

    "NumlockClear",
    "NumPad0", "NumPad1", "NumPad2", "NumPad3", "NumPad4", "NumPad5", "NumPad6", "NumPad7", "NumPad8", "NumPad9",
    "NumPadDivide", "NumPadMultiply", "NumPadSubtract", "NumPadPlus", "NumPadEnter", "NumPadDecimal", "NumPadComma",

    "Comma", "Decimal", "FwdSlash", "BackSlash", "SingleQuote", "SemiColon", "Equal", "Subtract", "BackQuote", "LBracket", "RBracket",

    "Exclaim", "At", "Hash", "Dollar", "Percent", "Caret", "Ampersand", "Asterisk", "LParen", "RParen", "Underscore",
    "Plus", "DblQuote", "Colon", "Question", "Greater", "Less",

    "Tab", "CapsLock", "LShift", "RShift", "LCtrl", "RCtrl", "RAlt", "LAlt", "RGui", "LGui",

    "Mute", "Stop", "PrevTrack", "PlayPause", "NextTrack", "VolumeUp", "VolumeDown",

    "UNKNOWN"
};

// Format Specifiers

template <>
struct fmt::formatter<KeyCode> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(KeyCode const& kc, Ctx& ctx) {
        return format_to(ctx.out(), "KeyCode::{}", KEYCODE_STR_MAP[static_cast<std::uint32_t>(kc)]);
    }
};

template <>
struct fmt::formatter<KeyboardInput> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(KeyboardInput const& ki, Ctx& ctx) {
        return format_to(
            ctx.out(),
            "KeyboardInput(scan_code: {}, key_code: {}, state: {}",
            ki.scan_code,
            ki.key_code,
            ki.state);
    }
};