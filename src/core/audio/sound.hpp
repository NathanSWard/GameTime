#pragma once

#include <AL/al.h>

struct SoundPlayOptions
{
    bool loop = false;
};

class Sound
{
    static constexpr ALuint null_buffer = 0;
    ALuint m_buffer;

    friend class AudioLoader;

    constexpr Sound(ALuint const buffer) noexcept : m_buffer(buffer) {}

public:
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

    ~Sound() noexcept { if (m_buffer != null_buffer) alDeleteBuffers(1, &m_buffer); }
};