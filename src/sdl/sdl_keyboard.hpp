#pragma once

#include <SDL2/SDL.h>
#include <core/input/keyboard.hpp>

void sdl_handle_keyboard_event(
    Events<KeyboardInput>& keyboard_events,
    SDL_KeyboardEvent const& e)
{
    auto const state = e.state == SDL_PRESSED ? ElementState::Pressed : ElementState::Released;

    if (e.repeat != 0) {
        return;
    }

    auto const key_code = [keycode = e.keysym.sym]() -> KeyCode {
        switch (keycode) {
            case SDLK_1: return KeyCode::Key1;
            case SDLK_2: return KeyCode::Key2;
            case SDLK_3: return KeyCode::Key3;
            case SDLK_4: return KeyCode::Key4;
            case SDLK_5: return KeyCode::Key5;
            case SDLK_6: return KeyCode::Key6;
            case SDLK_7: return KeyCode::Key7;
            case SDLK_8: return KeyCode::Key8;
            case SDLK_9: return KeyCode::Key9;
            case SDLK_0: return KeyCode::Key0;

            case SDLK_a: return KeyCode::A;
            case SDLK_b: return KeyCode::B;
            case SDLK_c: return KeyCode::C;
            case SDLK_d: return KeyCode::D;
            case SDLK_e: return KeyCode::E;
            case SDLK_f: return KeyCode::F;
            case SDLK_g: return KeyCode::G;
            case SDLK_h: return KeyCode::H;
            case SDLK_i: return KeyCode::I;
            case SDLK_j: return KeyCode::J;
            case SDLK_k: return KeyCode::K;
            case SDLK_l: return KeyCode::L;
            case SDLK_m: return KeyCode::M;
            case SDLK_n: return KeyCode::N;
            case SDLK_o: return KeyCode::O;
            case SDLK_p: return KeyCode::P;
            case SDLK_q: return KeyCode::Q;
            case SDLK_r: return KeyCode::R;
            case SDLK_s: return KeyCode::S;
            case SDLK_t: return KeyCode::T;
            case SDLK_u: return KeyCode::U;
            case SDLK_v: return KeyCode::V;
            case SDLK_w: return KeyCode::W;
            case SDLK_x: return KeyCode::X;
            case SDLK_y: return KeyCode::Y;
            case SDLK_z: return KeyCode::Z;

            case SDLK_ESCAPE: return KeyCode::Esc;

            case SDLK_F1: return KeyCode::F1;
            case SDLK_F2: return KeyCode::F2;
            case SDLK_F3: return KeyCode::F3;
            case SDLK_F4: return KeyCode::F4;
            case SDLK_F5: return KeyCode::F5;
            case SDLK_F6: return KeyCode::F6;
            case SDLK_F7: return KeyCode::F7;
            case SDLK_F8: return KeyCode::F8;
            case SDLK_F9: return KeyCode::F9;
            case SDLK_F10: return KeyCode::F10;
            case SDLK_F11: return KeyCode::F11;
            case SDLK_F12: return KeyCode::F12;
            case SDLK_F13: return KeyCode::F13;
            case SDLK_F14: return KeyCode::F14;
            case SDLK_F15: return KeyCode::F15;
            case SDLK_F16: return KeyCode::F16;
            case SDLK_F17: return KeyCode::F17;
            case SDLK_F18: return KeyCode::F18;
            case SDLK_F19: return KeyCode::F19;
            case SDLK_F20: return KeyCode::F20;
            case SDLK_F21: return KeyCode::F21;
            case SDLK_F22: return KeyCode::F22;
            case SDLK_F23: return KeyCode::F23;
            case SDLK_F24: return KeyCode::F24;

            case SDLK_PRINTSCREEN: return KeyCode::PrintScreen;
            case SDLK_SCROLLLOCK: return KeyCode::ScrollLock;
            case SDLK_PAUSE: return KeyCode::PauseBreak;

            case SDLK_INSERT: return KeyCode::Insert;
            case SDLK_HOME: return KeyCode::Home;
            case SDLK_DELETE: return KeyCode::Delete;
            case SDLK_END: return KeyCode::End;
            case SDLK_PAGEUP: return KeyCode::PageUp;
            case SDLK_PAGEDOWN: return KeyCode::PageDown;

            case SDLK_UP: return KeyCode::Up;
            case SDLK_DOWN: return KeyCode::Down;
            case SDLK_LEFT: return KeyCode::Left;
            case SDLK_RIGHT: return KeyCode::Right;

            case SDLK_BACKSPACE: return KeyCode::Backspace;
            case SDLK_RETURN: return KeyCode::Enter;
            case SDLK_SPACE: return KeyCode::Space;

            case SDLK_APPLICATION: return KeyCode::Application;
            case SDLK_SLEEP: return KeyCode::Sleep;

            case SDLK_NUMLOCKCLEAR: return KeyCode::NumlockClear;

            case SDLK_KP_0: return KeyCode::NumPad0;
            case SDLK_KP_1: return KeyCode::NumPad1;
            case SDLK_KP_2: return KeyCode::NumPad2;
            case SDLK_KP_3: return KeyCode::NumPad3;
            case SDLK_KP_4: return KeyCode::NumPad4;
            case SDLK_KP_5: return KeyCode::NumPad5;
            case SDLK_KP_6: return KeyCode::NumPad6;
            case SDLK_KP_7: return KeyCode::NumPad7;
            case SDLK_KP_8: return KeyCode::NumPad8;
            case SDLK_KP_9: return KeyCode::NumPad9;
            case SDLK_KP_DIVIDE: return KeyCode::NumPadDivide;
            case SDLK_KP_MULTIPLY: return KeyCode::NumPadMultiply;
            case SDLK_KP_MINUS: return KeyCode::NumPadSubtract;
            case SDLK_KP_PLUS: return KeyCode::NumPadPlus;
            case SDLK_KP_ENTER: return KeyCode::NumPadEnter;
            case SDLK_KP_PERIOD: return KeyCode::NumPadDecimal;
            case SDLK_KP_COMMA: return KeyCode::NumPadComma;

            case SDLK_COMMA: return KeyCode::Comma;
            case SDLK_PERIOD: return KeyCode::Decimal;
            case SDLK_SLASH: return KeyCode::FwdSlash;
            case SDLK_BACKSLASH: return KeyCode::BackSlash;
            case SDLK_QUOTE: return KeyCode::SingleQuote;
            case SDLK_SEMICOLON: return KeyCode::SemiColon;
            case SDLK_EQUALS: return KeyCode::Equal;
            case SDLK_MINUS: return KeyCode::Subtract;
            case SDLK_BACKQUOTE: return KeyCode::BackQuote;
            case SDLK_LEFTBRACKET: return KeyCode::LBracket;
            case SDLK_RIGHTBRACKET: return KeyCode::RBracket;

            case SDLK_EXCLAIM: return KeyCode::Exclaim;
            case SDLK_AT: return KeyCode::At;
            case SDLK_HASH: return KeyCode::Hash;
            case SDLK_DOLLAR: return KeyCode::Dollar;
            case SDLK_PERCENT: return KeyCode::Percent;
            case SDLK_CARET: return KeyCode::Caret;
            case SDLK_AMPERSAND: return KeyCode::Ampersand;

            case SDLK_ASTERISK: return KeyCode::Asterisk;
            case SDLK_LEFTPAREN: return KeyCode::LParen;
            case SDLK_RIGHTPAREN: return KeyCode::RParen;
            case SDLK_UNDERSCORE: return KeyCode::Underscore;
            case SDLK_PLUS: return KeyCode::Plus;
            case SDLK_QUOTEDBL: return KeyCode::DblQuote;
            case SDLK_COLON: return KeyCode::Colon;
            case SDLK_QUESTION: return KeyCode::Question;
            case SDLK_GREATER: return KeyCode::Greater;
            case SDLK_LESS: return KeyCode::Less;
                
            case SDLK_TAB: return KeyCode::Tab;
            case SDLK_CAPSLOCK: return KeyCode::CapsLock;
            case SDLK_LSHIFT: return KeyCode::LShift;
            case SDLK_RSHIFT: return KeyCode::RShift;
            case SDLK_LCTRL: return KeyCode::LCtrl;
            case SDLK_RCTRL: return KeyCode::RCtrl;
            case SDLK_RALT: return KeyCode::RAlt;
            case SDLK_LALT: return KeyCode::LAlt;
            case SDLK_RGUI: return KeyCode::RGui;
            case SDLK_LGUI: return KeyCode::LGui;

            case SDLK_AUDIOMUTE: return KeyCode::Mute;
            case SDLK_AUDIOSTOP: return KeyCode::Stop;
            case SDLK_AUDIOPREV: return KeyCode::PrevTrack;
            case SDLK_AUDIOPLAY: return KeyCode::PlayPause;
            case SDLK_AUDIONEXT: return KeyCode::NextTrack;
            case SDLK_VOLUMEUP: return KeyCode::VolumeUp;
            case SDLK_VOLUMEDOWN: return KeyCode::VolumeDown;

            default: return KeyCode::UNKNOWN;
        }
    }();

    keyboard_events.send(KeyboardInput{
        .scan_code = static_cast<std::uint32_t>(e.keysym.scancode),
        .key_code = key_code,
        .state = ElementState{.state = state},
        });
}