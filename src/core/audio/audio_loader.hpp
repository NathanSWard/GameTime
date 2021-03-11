#pragma once

#include <AL/al.h>
#include <AL/alext.h>
#include <core/audio/audio.hpp>
#include <core/audio/error.hpp>
#include <core/assets/loader.hpp>
#include <debug/debug.hpp>
#include <sndfile.h>

namespace {

    struct UserData
    {
        std::span<std::byte> bytes;
        std::size_t pos = 0;
    };

    auto get_filelen(void* const user_data) -> sf_count_t
    {
        return static_cast<UserData*>(user_data)->bytes.size();
    }

    auto seek(sf_count_t const offset, int const whence, void* const user_data) -> sf_count_t
    {
        auto* const ud = static_cast<UserData*>(user_data);

        switch (whence) {
            case SEEK_CUR: 
                ud->pos = offset;
                break;
            case SEEK_SET: 
                ud->pos = 0;
                break;
            case SEEK_END: 
                ud->pos = ud->bytes.size();
                break;
            default: break;
        }

        return ud->pos;
    }

    auto read(void* const ptr, sf_count_t const count, void* const user_data) -> sf_count_t
    {
        auto* const ud = static_cast<UserData*>(user_data);
        auto const buffer_size = ud->bytes.size() - ud->pos;
        auto const requested_bytes = static_cast<std::size_t>(count);

        auto const bytes_to_read = requested_bytes < buffer_size ? requested_bytes : buffer_size;
        std::memcpy(ptr, static_cast<void const*>(ud->bytes.data() + ud->pos), bytes_to_read);
        ud->pos += bytes_to_read;
        return bytes_to_read;
    }

    auto write(void const* const ptr, sf_count_t const count, void* const user_data) -> sf_count_t
    {
        auto* const ud = static_cast<UserData*>(user_data);
        auto const buffer_size = ud->bytes.size() - ud->pos;
        auto const requested_bytes = static_cast<std::size_t>(count);

        auto const bytes_to_write = requested_bytes < buffer_size ? requested_bytes : buffer_size;
        std::memcpy(static_cast<void*>(ud->bytes.data() + ud->pos), ptr, bytes_to_write);
        ud->pos += bytes_to_write;
        return bytes_to_write;
    }

    auto tell(void* const user_data) -> sf_count_t
    {
        return static_cast<UserData*>(user_data)->pos;
    }

} // namespace

struct AudioLoader final : public AssetLoader
{
    static constexpr auto exts = std::array<std::string_view, 2>{ "wav", "ogg" };

    auto extensions() const noexcept -> std::span<std::string_view const> final
    {
        return std::span<std::string_view const>{ exts.data(), exts.size() };
    }

    auto load(std::filesystem::path const&, std::span<std::byte> const bytes) const -> tl::optional<LoadedAsset> final
    {
        auto ud_bytes = bytes;
        auto ud = UserData {
            .bytes = ud_bytes,
            .pos = 0,
        };

        std::size_t io_pos = 0;
        auto vio = SF_VIRTUAL_IO {
            .get_filelen = get_filelen,
            .seek = seek,
            .read = read,
            .write = write,
            .tell = tell,
        };

        auto info = SF_INFO{};
        auto* const file = sf_open_virtual(&vio, SFM_READ, &info, static_cast<void*>(&ud));

        if (file == nullptr) {
            LOG_ERROR("sf_open_virtual failed. Error: {}", sf_strerror(file));
            return {};
        }

        constexpr auto short_per_int = static_cast<sf_count_t>(std::numeric_limits<int>::max() / sizeof(short));
        if (info.frames < 1 || info.frames >  short_per_int / info.channels) {
            LOG_ERROR("SF_INFO contains invalid frames");
            sf_close(file);
            return {};
        }

        auto const format = [&] {
            switch (info.channels) {
                case 1: 
                    return AL_FORMAT_MONO16;
                case 2:
                    return AL_FORMAT_STEREO16;
                case 3: {
                    if (sf_command(file, SFC_WAVEX_GET_AMBISONIC, nullptr, 0) == SF_AMBISONIC_B_FORMAT) {
                        return AL_FORMAT_BFORMAT2D_16;
                    }
                    return AL_NONE;
                }
                case 4: {
                    if (sf_command(file, SFC_WAVEX_GET_AMBISONIC, nullptr, 0) == SF_AMBISONIC_B_FORMAT) {
                        return AL_FORMAT_BFORMAT3D_16;
                    }
                    return AL_NONE;
                }
                default: 
                    return AL_NONE;
            }
        }();

        if (format == AL_NONE) {
            LOG_ERROR("SF_INFO contains invalid channels format");
            sf_close(file);
            return {};
        }

        auto const buffer_size = static_cast<std::size_t>(info.frames * info.channels);
        auto membuffer = std::vector<short>(buffer_size);

        auto const num_frames = sf_readf_short(file, membuffer.data(), info.frames);
        if (num_frames != info.frames) {
            LOG_ERROR("sf_readf_short failed to read '{}' frames. Only '{}' were read.", info.frames, num_frames);
            sf_close(file);
            return {};
        }

        sf_close(file);

        auto const num_bytes = static_cast<ALsizei>(num_frames * info.channels) * static_cast<ALsizei>(sizeof(short));

        ALuint buffer{};

        alGenBuffers(1, &buffer);
        if (auto const err = AudioError::current(); !err.is_ok()) {
            LOG_ERROR("alGenBuffers failed. Error: {}", err.to_string());
            return {};
        }


        alBufferData(buffer, format, membuffer.data(), num_bytes, info.samplerate);
        if (auto const err = AudioError::current(); !err.is_ok()) {
            LOG_ERROR("alGenBuffers failed. Error: {}", err.to_string());
            return {};
        }

        return LoadedAsset::create<Sound>(buffer);
    }
};