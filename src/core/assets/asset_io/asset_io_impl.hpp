#pragma once

#include "asset_io.hpp"
#include <cstdio>
#include <filesystem>
#include <util/common.hpp>


class FileAssetIo final : public AssetIo
{
    std::filesystem::path m_root_path;
    
public:
    explicit FileAssetIo(std::string root_path)
        : m_root_path(MOV(root_path))
    {}

    auto load_path(std::string_view const path) const -> std::function<Result()> final
    {
        auto const full_path = m_root_path / std::filesystem::path{ path };
        return [path = MOV(full_path)] () -> Result {
            FILE* file = nullptr;
            auto const err = fopen_s(&file, path.string().c_str(), "rb");
            if (err != 0) {
                return tl::make_unexpected(Error::IoError);
            }

            std::fseek(file, 0, SEEK_END);
            auto const size = std::ftell(file);
            std::rewind(file);

            auto buffer = std::vector<std::byte>(size, std::byte{});

            if (std::fread(buffer.data(), sizeof(std::byte), size, file) != size) {
                std::fclose(file);
                return tl::make_unexpected(Error::IoError);
            }

            std::fclose(file);
            return buffer;
        };
    }

    auto root_path() const noexcept -> std::filesystem::path final { return m_root_path;  }

};