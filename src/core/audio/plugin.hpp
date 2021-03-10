#pragma once

#include <core/game/game.hpp>
#include <core/audio/audio.hpp>
#include <core/audio/audio_loader.hpp>

struct AudioPlugin
{
    void build(GameBuilder& builder)
    {
        auto context = AudioContext::create();
        if (!context) {
            panic("AudioContext failed to initialize");
            return;
        }

        auto const audio_output_settings = builder
            .resources()
            .get_resource<AudioOutputSettings>()
            .map([](auto const& r) { return *r; })
            .value_or(AudioOutputSettings{});

        auto audio_output = AudioOutput::create(audio_output_settings);
        if (!audio_output) {
            panic("AudioOuput failed to initialize");
            return;
        }

        builder
            .set_resource<AudioContext>(*MOV(context))
            .set_resource<AudioOutput>(*MOV(audio_output))
            .set_resource<Audio>()
            .add_asset<Sound>()
            .add_asset_loader<AudioLoader>()
            .add_system_to_stage<CoreStages::PostUpdate>(play_queued_audio_system);
    }
};