#pragma once

#include <core/game.hpp>
#include "input.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"

struct InputPlugin 
{
    void build(Game& game) const
    {
        game.add_event<KeyboardInput>()
            .add_event<MouseButtonInput>()
            .add_event<MouseMotion>()
            .add_event<MouseWheel>()
            .add_system_to_stage<CoreStages::Events>(keyboard_input_system)
            .add_system_to_stage<CoreStages::Events>(mouse_button_input_system);
    }
};