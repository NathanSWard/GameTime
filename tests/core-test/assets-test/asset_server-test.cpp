#include <ut.hpp>
#include <core/assets/asset_server.hpp>
#include <atomic>

using namespace boost::ut;

std::vector<std::byte> const gbytes = std::vector{ 10, std::byte{} };

struct TestAssetIo final : public AssetIo
{
    auto load_path(std::filesystem::path const&) const -> std::function<Result()> final
    {
        return []() -> Result { return gbytes; };
    }

    auto root_path() const noexcept -> std::filesystem::path final { return std::filesystem::path("."); }
};

struct FailingTestAssetIo final : public AssetIo
{
    auto load_path(std::filesystem::path const&) const -> std::function<Result()> final
    {
        return []() -> Result { return tl::make_unexpected(Error::IoError); };
    }

    auto root_path() const noexcept -> std::filesystem::path final { return std::filesystem::path("."); }
};

constexpr std::string_view asset_ext = "hello";
constexpr std::string_view asset_path = "a/b/c/file.hello";
constexpr std::string_view asset_path2 = "a/b/sup.hello";

struct TestAsset {};

struct TestAssetLoader final : AssetLoader
{
    static constexpr auto exts = std::array<std::string_view, 1>{ asset_ext };
    inline static std::atomic<int> times_loaded{ 0 };
    
    auto extensions() const noexcept -> std::span<std::string_view const> final
    {
        return std::span{ exts.data(), 1 };
    }

    auto load(std::filesystem::path const&, std::span<std::byte> bytes) const -> tl::optional<LoadedAsset> final
    {
        ++times_loaded;
        return LoadedAsset::create<TestAsset>();
    }
};

struct FailingTestAssetLoader final : AssetLoader
{
    static constexpr auto exts = std::array<std::string_view, 1>{ asset_ext };

    auto extensions() const noexcept -> std::span<std::string_view const> final
    {
        return std::span{ exts.data(), 1 };
    }

    auto load(std::filesystem::path const&, std::span<std::byte> bytes) const -> tl::optional<LoadedAsset> final
    {
        return tl::nullopt;
    }
};

void asset_server_test()
{
    "[AssetServer]"_test = [] {
        auto server = AssetServer(std::make_unique<TestAssetIo>(), TaskPool{});

        auto assets = server.register_asset_type<TestAsset>();

        server.add_asset_loader<TestAssetLoader>();

        should("contains asset loader for extension") = [&] {
            expect(server.get_asset_loader_from_extension(asset_ext).has_value());
            expect(server.get_asset_loader_from_path(asset_path).has_value());
            expect(!server.get_asset_loader_from_extension("png").has_value());
            expect(!server.get_asset_loader_from_path("picture.png").has_value());
        };

        expect(assets.empty());
        server.update_assets(assets);
        expect(assets.empty());
        
        {
            auto handle = assets.add_asset();
            should("not have load state") = [&] {
                expect(server.get_load_state(handle) == LoadState::NotLoaded);
                expect(server.get_load_state(handle.untyped()) == LoadState::NotLoaded);
            };
            assets.remove_asset(handle);
        }

        should("fail loading missing extension") = [&] {
            auto const result = server.load_sync("picture.png");
            expect((!result.has_value()) >> fatal);
            expect(result.error() == AssetServer::Error::MissingAssetLoader);
        };

        should("succeed loading valid extension") = [&] {
            auto const result = server.load_sync(asset_path);
            expect((result.has_value()) >> fatal);
            
            auto path_id = *result;
            expect(server.get_load_state(UntypedHandle{ HandleId { path_id } }) == LoadState::Loaded);

            server.update_assets(assets);
            expect(assets.size() == 1);

            should("reloading the same asset does `nothing`") = [&] {
                auto const same_result = server.load_sync(asset_path);
                expect((same_result.has_value()) >> fatal);
                expect(*result == *same_result);
                expect(assets.size() == 1);
            };

            auto const handle = server.load<TestAsset>(asset_path2);
            for (;;) { // if loop is not infinite, the test pasts
                server.update_assets(assets);
                if (assets.size() == 2) {
                    break;
                }
                std::this_thread::yield();
            }
            expect(server.get_load_state(handle) == LoadState::Loaded);
        };
    };

    "[AssetServer]: Failing Io"_test = [] {
        auto server = AssetServer(std::make_unique<FailingTestAssetIo>(), TaskPool{});
        auto assets = server.register_asset_type<TestAsset>();
        server.add_asset_loader<TestAssetLoader>();

        auto const result = server.load_sync(asset_path);
        expect((!result.has_value()) >> fatal);
        expect(result.error() == AssetServer::Error::AssetIoError);
    };

    "[AssetServer]: Failing Loader"_test = [] {
        auto server = AssetServer(std::make_unique<TestAssetIo>(), TaskPool{});
        auto assets = server.register_asset_type<TestAsset>();
        server.add_asset_loader<FailingTestAssetLoader>();

        should("loader fail") = [&] {
            auto const result = server.load_sync(asset_path);
            expect((!result.has_value()) >> fatal);
            expect(result.error() == AssetServer::Error::AssetLoaderError);
        };

        should("replace loader") = [&] {
            // replace loader
            server.add_asset_loader<TestAssetLoader>();
            auto const result = server.load_sync(asset_path);
            expect(result.has_value());
        };
    };
}