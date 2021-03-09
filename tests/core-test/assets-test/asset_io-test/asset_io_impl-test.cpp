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
        
        { // load a single file
            auto result = io.load_path("pngs/png-image.png")();
            expect(result.has_value());
        }

        { // load a directory
            auto result = io.read_directory("pngs");
            expect((result.has_value()) >> fatal);

            int num_paths = 0;
            for (auto const& entry : *result) {
                ++num_paths;
                expect(entry.path() == fs::path{ "assets/pngs/png-image.png" });
            }
            expect(num_paths == 1);
        }
    };
}