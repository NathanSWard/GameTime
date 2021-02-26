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
    virtual auto load_path(std::string_view path) const -> std::function<Result()> = 0;
    virtual auto root_path() const noexcept -> std::filesystem::path = 0;
};

template <typename T>
concept IsAssetIo = std::is_base_of_v<AssetIo, T>;
