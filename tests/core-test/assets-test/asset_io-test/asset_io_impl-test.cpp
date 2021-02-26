#include <ut.hpp>
#include <core/assets/asset_io/asset_io_impl.hpp>
#include <span>

using namespace boost::ut;
namespace fs = std::filesystem;

auto write_file(fs::path const& path, std::span<std::byte const> const bytes) -> bool
{
    auto file = std::fopen(path.string().c_str(), "w");
    if (file == nullptr) {
        return false;
    }

    bool ok = true;
    auto const size = bytes.size();
    if (std::fwrite(bytes.data(), sizeof(std::byte), size, file) != size) {
        ok = false;
    }
    
    std::fclose(file);

    return ok;
}

auto remove_file(fs::path const& path) -> bool
{
    return std::remove(path.string().c_str()) == 0;
}

void asset_io_impl_test()
{
    auto const io = FileAssetIo("");
    expect(io.root_path() == fs::path{ "" });

    auto const bytes = std::vector<std::byte>{ 10, std::byte{} };

    auto const path = fs::path{ "test-file.txt" };

    if (!write_file(path, bytes)) {
        remove_file(path);
        expect((false) >> fatal);
    }

    auto result = io.load_path(path.string())();

    if (!result.has_value()) {
        remove_file(path);
        expect((false) >> fatal);
    }

    expect(*result == bytes);
    remove_file(path);
}