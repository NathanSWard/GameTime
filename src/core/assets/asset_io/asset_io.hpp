#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <future>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

struct AssetIo
{
    enum class Error
    {
        NotFound,
        IoError,
    };

    using Result = tl::expected<std::vector<std::byte>, Error>;

    // TODO: This should probably return a std::function<Result(std::string_view)>
    //       Or maybe a boost::future??
    virtual auto load_path(std::filesystem::path const& path) const -> std::function<Result()> = 0;
    virtual auto root_path() const noexcept -> std::filesystem::path = 0;

    virtual auto is_directory(std::filesystem::path const& path) const noexcept -> bool 
    {
        std::error_code ec{};
        return std::filesystem::is_directory(path, ec);
    }

    virtual auto read_directory(std::filesystem::path const& dir) const noexcept -> tl::expected<std::filesystem::directory_iterator, std::error_code>
    {
        std::error_code ec{};
        auto dir_iter = std::filesystem::directory_iterator(root_path() / dir, ec);
        if (ec != std::error_code{}) {
            return tl::make_unexpected(ec);
        }
        return dir_iter;
    }
};

template <typename T>
concept IsAssetIo = std::is_base_of_v<AssetIo, T>;
