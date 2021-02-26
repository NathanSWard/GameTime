#pragma once

#include <cstddef>
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

    virtual auto load_path(std::string_view path) const -> std::future<Result> = 0;
};

template <typename T>
concept IsAssetIo = std::is_base_of_v<AssetIo, T>;
