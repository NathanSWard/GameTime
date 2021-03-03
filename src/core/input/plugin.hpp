#pragma once

#include <core/game/game.hpp>
#include "input.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"

struct InputPlugin 
{
    void build(GameBuilder& builder) const
    {
        builder
            .add_event<KeyboardInput>()
            .set_resource<Input<KeyCode>>()
            .add_event<MouseButtonInput>()
            .set_resource<Input<MouseButton>>()
            .add_event<MouseMotion>()
            .add_event<MouseWheel>()
            .add_system_to_stage<CoreStages::Events>(keyboard_input_system)
            .add_system_to_stage<CoreStages::Events>(mouse_button_input_system);
    }
};