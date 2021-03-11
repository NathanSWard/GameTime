#pragma once

#include <AL/al.h>
#include <core/audio/error.hpp>
#include <core/audio/sound.hpp>
#include <util/common.hpp>
#include <tl/expected.hpp>

struct ChannelId
{
    std::size_t id;
};

struct ChannelSettings
{
    float volume = 1.f;
};

class Channel
{
    ALuint m_source;
    bool m_is_ok = true;

    constexpr Channel(ALuint const source) noexcept
        : m_source(source)
        , m_is_ok(true)
    {}

public:
    constexpr Channel(Channel&& other) noexcept
        : m_source(other.m_source)
        , m_is_ok(std::exchange(other.m_is_ok, false))
    {}

    ~Channel() noexcept { if (m_is_ok) alDeleteSources(1, &m_source); }

    auto create(ChannelSettings const& settings) -> tl::expected<Channel, AudioError>
    {
        ALuint source{};
        alGenSources(1, &source);
        if (auto const err = AudioError::current(); !err.is_ok()) {
            return tl::make_unexpected(err);
        }

        alSourcef(source, AL_GAIN, settings.volume);
        if (auto const err = AudioError::current(); !err.is_ok()) {
            return tl::make_unexpected(err);
        }

        return Channel(source, settings);
    }

    auto volume() const noexcept -> float 
    {
        float volume{};
        return alGetSourcef(m_source, AL_GAIN, &volume);
        return volume;
    }

    void set_volume(float const volume) noexcept
    {
        alSourcef(m_source, AL_GAIN, volume);
    }
};

// format specifiers

template <>
struct fmt::formatter<ChannelId> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(ChannelId const& ci, Ctx& ctx) {
        return format_to(ctx.out(), "ChannelId(id: {})", ci.id);
    }
};