#include <ut.hpp>
#include <core/assets/asset_io/asset_io_impl.hpp>
#include <span>

using namespace boost::ut;
namespace fs = std::filesystem;

void asset_io_impl_test()
{
    "[FileAssetIo]"_test = [] {
        auto const io = FileAssetIo("assets");
        expect(io.root_path() == fs::path{ "assets" });
        auto result = io.load_path(fs::path{ "png-image.png" }.string())();
        expect(result.has_value());
    };
}