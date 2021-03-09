#include <ut.hpp>
#include <core/assets/asset_io/asset_io_impl.hpp>
#include <sdl/sdl_img_loader.hpp>
#include <filesystem>

#include <errno.h>

using namespace boost::ut;
namespace fs = std::filesystem;

constexpr auto flags = IMG_INIT_PNG;

struct IMGContext {
    int result = 0;
    IMGContext() { result = IMG_Init(flags); }
    ~IMGContext() { IMG_Quit(); }
};

void sdl_img_loader_test()
{
    "[SDL_IMG_Loader]"_test = [] {
        auto context = IMGContext{};
        expect(((context.result & flags) == flags) >> fatal);

        auto loader = SDL_IMG_Loader();
        auto const io = FileAssetIo("assets");

        should("load png") = [&] {
            auto bytes = io.load_path("pngs/png-image.png")();
            expect((bytes.has_value()) >> fatal);

            auto const path = fs::path{ "assets/pngs/png-image.png" };
            auto const png = loader.load(path, *bytes);
            expect((png.has_value()) >> fatal);
            expect(png->type_id() == type_id<Texture>());
        };
    };
}