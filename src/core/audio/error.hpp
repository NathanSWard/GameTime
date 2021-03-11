#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <fmt/format.h>

struct AudioError
{
    ALenum value = static_cast<ALenum>(AL_NO_ERROR);

    static constexpr auto current() noexcept -> AudioError
    {
        return AudioError{ .value = alGetError() };
    }

    constexpr auto to_string() const noexcept -> std::string_view
    {
        switch (value) {
            case AL_NO_ERROR: return "No error";
            case AL_INVALID_NAME: return "Invalid Name";
            case AL_INVALID_ENUM: return "Invalid Enum";
            case AL_INVALID_VALUE: return "Invalid Value";
            case AL_INVALID_OPERATION: return "Invalid Operation";
            case AL_OUT_OF_MEMORY: return "Out of memory";
            default: return "Unknown";
        }
    }

    constexpr auto is_ok() const noexcept -> bool
    {
        return value == static_cast<ALenum>(AL_NO_ERROR);
    }
};

template <>
struct fmt::formatter<AudioError> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Ctx>
    auto format(AudioError const& ae, Ctx& ctx) {
        return format_to(ctx.out(), "AudioError('{}')", ae.to_string());
    }
};
