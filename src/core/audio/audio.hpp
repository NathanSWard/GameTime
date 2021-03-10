#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <core/assets/handle.hpp>
#include <debug/debug.hpp>
#include <queue>
#include <spdlog/spdlog.h>
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
    constexpr AudioContext(AudioContext&& other) noexcept
        : m_device(std::exchange(other.m_device, nullptr))
        , m_ctx(std::exchange(other.m_ctx, nullptr))
    {}

    static auto create() -> tl::optional<AudioContext>
    {
        auto* const device = alcOpenDevice(nullptr);
        if (!device) {
            spdlog::error("alcOpenDevice() failed.");
            return {};
        }

        auto* const ctx = alcCreateContext(device, nullptr);
        if (!ctx) {
            spdlog::error("alcCreateContext() failed. Error: {}", alcGetError(device));
            alcCloseDevice(device);
            return {};
        }

        if (!alcMakeContextCurrent(ctx)) {
            spdlog::error("alcMakeContextCurrent() failed. Error: {}", alcGetError(device));
            return {};
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

class Sound
{
    static constexpr ALuint null_buffer = 0;
    ALuint m_buffer;

    friend class AudioOutput;

public:
    constexpr Sound(ALuint const buffer) noexcept : m_buffer(buffer) {}

    constexpr Sound(Sound&& other) noexcept
        : m_buffer(std::exchange(other.m_buffer, null_buffer))
    {}

    Sound& operator=(Sound&& other) noexcept
    {
        if (m_buffer != null_buffer) {
            alDeleteBuffers(1, &m_buffer);
        }

        m_buffer = std::exchange(other.m_buffer, null_buffer);
        return *this;
    }

    ~Sound() { if (m_buffer != null_buffer) alDeleteBuffers(1, &m_buffer); }
};

struct SoundOptions
{
    bool loop = false;
};

struct Audio
{
    RwLock<std::queue<std::tuple<Handle<Sound>, SoundOptions>>> queued_sounds;

    void play(Handle<Sound> handle, SoundOptions options = SoundOptions{})
    {
        queued_sounds.write()->emplace(MOV(handle), MOV(options));
    }
};

struct AudioOutputSettings
{
    std::uint8_t max_channels = 16;
};

class AudioOutput
{
    std::vector<ALuint> m_sources;

    AudioOutput(std::vector<ALuint>&& sources) noexcept
        : m_sources(MOV(sources))
    {}

    void play_sound(Sound const& sound, SoundOptions const& options) const noexcept
    {
        // TODO: checking for playing sources and either override or find a not playing source
        alSourcei(m_sources[0], AL_BUFFER, static_cast<ALint>(sound.m_buffer));
        alSourcePlay(m_sources[0]);
    }

public:
    AudioOutput(AudioOutput&& other) noexcept
        : m_sources(std::exchange(other.m_sources, std::vector<ALuint>{}))
    {}

    ~AudioOutput()
    {
        alDeleteSources(static_cast<ALsizei>(m_sources.size()), m_sources.data());
    }

    static auto create(AudioOutputSettings const& settings) -> tl::optional<AudioOutput>
    {
        auto sources = std::vector<ALuint>(settings.max_channels);
        alGenSources(static_cast<ALsizei>(sources.size()), sources.data());

        if (auto const error = alGetError(); error != AL_NO_ERROR) {
            LOG_ERROR("Unable to cerate Audio Channel. Error: {}", error);
            return {};
        }

        return AudioOutput(MOV(sources));
    }

    void try_play_queued(Assets<Sound> const& sounds, Audio& audio) const
    {
        auto queue = audio.queued_sounds.write();
        auto const queue_size = queue->size();
        for (std::size_t i = 0; i < queue_size; ++i) {
            auto [handle, options] = MOV(queue->front());
            queue->pop();

            if (auto asset = sounds.get_asset(handle); asset) {
                play_sound(*asset, options);
            }
            else {
                queue->emplace(MOV(handle), MOV(options));
            }
        }
    }
};

void play_queued_audio_system(
    Resource<AudioOutput> audio_output, 
    Resource<Assets<Sound>> sounds,
    Resource<Audio> audio)
{
    if (audio->queued_sounds.read()->empty()) {
        return;
    }
    audio_output->try_play_queued(*sounds, *audio);
}