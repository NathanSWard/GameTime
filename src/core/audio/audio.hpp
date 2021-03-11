#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <core/audio/sound.hpp>
#include <core/assets/handle.hpp>
#include <debug/debug.hpp>
#include <queue>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <util/sync/rwlock.hpp>

class AudioContext
{
    struct ALCdevice* m_device = nullptr;
    struct ALCcontext* m_ctx = nullptr;

    constexpr AudioContext(ALCdevice* const device, ALCcontext* const ctx) noexcept
        : m_device(device)
        , m_ctx(ctx)
    {}

public:
    struct Error
    {
        ALenum value;

        constexpr auto to_string() const noexcept -> std::string_view
        {
            switch (value) {
                case ALC_NO_ERROR: return "No error";
                case ALC_INVALID_DEVICE: return "Invalid Device";
                case ALC_INVALID_CONTEXT: return "Invalid Context";
                case ALC_INVALID_ENUM: return "Invalid Enum";
                case ALC_INVALID_VALUE: return "Invalid Value";
                case ALC_OUT_OF_MEMORY: return "Out of memory";
                default: return "Unknown";
            }
        }

        static auto current(ALCdevice* const device) noexcept -> Error
        {
            return Error{ .value = alcGetError(device) };
        }
    };

    constexpr AudioContext(AudioContext&& other) noexcept
        : m_device(std::exchange(other.m_device, nullptr))
        , m_ctx(std::exchange(other.m_ctx, nullptr))
    {}

    static auto create() -> tl::expected<AudioContext, Error>
    {
        auto* const device = alcOpenDevice(nullptr);
        if (!device) {
            return tl::make_unexpected(Error::current(device));
        }

        auto* const ctx = alcCreateContext(device, nullptr);
        if (!ctx) {
            return tl::make_unexpected(Error::current(device));
        }

        if (!alcMakeContextCurrent(ctx)) {
            return tl::make_unexpected(Error::current(device));
        }

        return AudioContext(device, ctx);
    }

    ~AudioContext()
    {
        if (m_ctx && m_device) {
            alcDestroyContext(m_ctx);
            alcCloseDevice(m_device);
        }
    }
};

struct Audio
{
    RwLock<std::queue<std::tuple<Handle<Sound>, SoundOptions>>> queued_sounds;

    void play(Handle<Sound> handle, SoundOptions options = SoundOptions{})
    {
        queued_sounds.write()->emplace(MOV(handle), MOV(options));
    }
};