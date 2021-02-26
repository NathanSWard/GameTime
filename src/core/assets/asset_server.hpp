#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <tl/expected.hpp>
#include <unordered_map>
#include <vector>

#include <debug/debug.hpp>
#include <util/common.hpp>
#include <util/sync/rwlock.hpp>
#include <core/task/task_pool.hpp>

#include "assets.hpp"
#include "asset_io.hpp"
#include "handle.hpp"
#include "loader.hpp"

enum class LoadState
{
    NotLoaded,
    Loading,
    Loaded,
    Failed,
};

struct AssetInfo
{
    std::string             path;
    LoadState               load_state = LoadState::NotLoaded;
    tl::optional<type_id_t> type_id;
    std::size_t             version = 0;
};

namespace {
    struct StoredAsset
    {
        void_ptr data;
        AssetPathId path_id;
    };

    struct AssetServerInternal
    {
        AssetServerInternal(std::unique_ptr<AssetIo> asset_io, TaskPool taskpool)
            : asset_io(MOV(asset_io))
            , task_pool(MOV(taskpool))
        {}

        TaskPool task_pool;
        std::unique_ptr<AssetIo> asset_io;
        RwLock<std::vector<std::shared_ptr<AssetLoader>>> loaders;
        RwLock<std::unordered_map<std::string, std::size_t, hash::string_hash, std::equal_to<>>> extension_to_loader_index;
        RwLock<std::unordered_map<AssetPathId, AssetInfo>> asset_info;
        RwLock<std::unordered_map<type_id_t, std::vector<StoredAsset>>> stored_assets;
    };
}

// TODO: Ref count each handle per asset
class AssetServer
{
    std::shared_ptr<AssetServerInternal> m_internal;

public:

    enum Error
    {
        MissingAssetLoader,
        IncorrectHandleType,
        AssetLoaderError,
        AssetIoError,
    };

    template <typename T>
    using AssetServerResult = tl::expected<T, Error>;

    AssetServer(std::unique_ptr<AssetIo> asset_io, TaskPool taskpool) noexcept
        : m_internal(std::make_shared<AssetServerInternal>(MOV(asset_io), MOV(taskpool)))
    {
        DEBUG_ASSERT(m_internal->asset_io != nullptr);
    }

    AssetServer(AssetServer&&) noexcept = default;
    AssetServer& operator=(AssetServer&&) noexcept = default;
    AssetServer(AssetServer const&) noexcept = default;
    AssetServer& operator=(AssetServer const&) noexcept = default;

    template <typename T>
    [[nodiscard]] auto register_asset_type() const -> Assets<T>
    {
        auto stored_assets = m_internal->stored_assets.write();
        stored_assets->emplace(std::piecewise_construct,
            std::forward_as_tuple(type_id<T>()),
            std::forward_as_tuple()
        );
        return Assets<T>{};
    }

    template <IsAssetLoader T, typename... Args>
    void add_asset_loader(Args&&... args)
    {
        auto loader = std::make_unique<T>(FWD(args)...);

        auto loaders = m_internal->loaders.write();
        auto extension_map = m_internal->extension_to_loader_index.write();

        auto const index = loaders->size();
        for (auto const extension : loader->extensions()) {
            extension_map->insert_or_assign(std::string{ extension }, index);
        }
        loaders->push_back(MOV(loader));
    }

    [[nodiscard]] auto get_asset_loader_from_extension(std::string_view const extension) const -> tl::optional<std::shared_ptr<AssetLoader>>
    {
        auto index_map = m_internal->extension_to_loader_index.read();
        if (auto const iter = index_map->find(extension); iter != index_map->end()) {
            auto const index = iter->second;
            auto loaders = m_internal->loaders.read();
            return tl::make_optional((*loaders)[index]);
        }
        return {};
    }

    [[nodiscard]] auto get_asset_loader_from_path(std::string_view const path) const -> tl::optional<std::shared_ptr<AssetLoader>>
    {
        // TODO: not the most efficent :(
        auto filename = std::filesystem::path(path.begin(), path.end()).filename().string();
        if (filename.empty()) {
            return {};
        }

        std::string_view ext = filename;
        for (;;) {
            auto const idx = ext.find_first_of('.');
            if (idx == std::string_view::npos) {
                break;
            }

            ext = ext.substr(idx + 1);
            if (auto const loader = get_asset_loader_from_extension(ext); loader.has_value()) {
                return loader;
            }
        }
        return {};
    }

    auto get_load_state(UntypedHandle const& handle) const -> LoadState
    {
        if (!handle.id().m_is_path_id) {
            return LoadState::NotLoaded;
        }

        auto asset_info = m_internal->asset_info.read();
        if (auto const iter = asset_info->find(handle.id().m_path_id); iter != asset_info->end()) {
            return iter->second.load_state;
        }
        return LoadState::NotLoaded;
    }

    template <typename T>
    auto get_load_state(Handle<T> const& handle) const -> LoadState
    {
        return get_load_state(handle.untyped());
    }

    // TODO: Make async??
    [[nodiscard]] auto load_sync(std::string_view const path) const -> AssetServerResult<AssetPathId>
    {
        auto loader = get_asset_loader_from_path(path);
        if (!loader) {
            return tl::make_unexpected(Error::MissingAssetLoader);
        }

        auto const path_id = HandleId::from_path(path).m_path_id;

        auto const version = [&] {
            auto asset_info = m_internal->asset_info.write();
            
            auto const [iter, inserted] = 
                asset_info->emplace(std::piecewise_construct,
                    std::forward_as_tuple(path_id), 
                    std::forward_as_tuple( // AssetPathInfo
                        std::string{ path }, // path
                        LoadState::NotLoaded, // load_state
                        tl::nullopt, // type_id
                        0 // version
                    ));

            // TODO: Possibly give the option to force the asset to reload?
            //       Also, if the asset is already loaded, should we remove the info?
            if (!inserted) {// asset already exists
                return path_id;
            }

            auto& info = iter->second;
            info.load_state = LoadState::Loading;
            info.version += 1;
            return info.version;
        }();

        auto set_load_state = [&](LoadState const state) {
            auto asset_info = m_internal->asset_info.write();
            auto info = asset_info->find(path_id);
            DEBUG_ASSERT(info != asset_info->end());
            info->second.load_state = state;
        };

        // load the asset file's bytes
        auto bytes = m_internal->asset_io->load_path(path).get();
        if (!bytes) {
            set_load_state(LoadState::Failed);
            return tl::make_unexpected(Error::AssetIoError);
        }

        // loaded the asset from the asset file's bytes
        auto loaded_asset = (*loader)->load(path, *bytes);
        if (!loaded_asset) {
            set_load_state(LoadState::Failed);
            return tl::make_unexpected(Error::AssetLoaderError);
        }

        // Check if version has changed since we last had the lock. 
        // Return if a newer version is being loaded.
        {
            auto asset_info = m_internal->asset_info.write();
            auto info = asset_info->find(path_id);
            DEBUG_ASSERT(info != asset_info->end());

            if (version != info->second.version) {
                return path_id;
            }

            info->second.type_id = tl::make_optional(loaded_asset->type_id());
            info->second.load_state = LoadState::Loaded;
        }

        // store the loaded asset
        auto stored_assets = m_internal->stored_assets.write();
        auto [assets, unused] = stored_assets->emplace(
            std::piecewise_construct,
            std::forward_as_tuple(loaded_asset->type_id()),
            std::forward_as_tuple()
        );
        UNUSED(unused);
        assets->second.push_back(StoredAsset{
            .data = MOV(loaded_asset->m_data),
            .path_id = path_id,
            });

        return path_id;
    }

    [[nodiscard]] auto load_untracked(std::string_view const path) const -> HandleId
    {
        auto const future = m_internal->task_pool.execute([server = *this, path = std::string(path)]{
            if (auto const result = server.load_sync(path); !result) {
                ;// LOG THE ERROR!!!
            }
            });
        UNUSED(future);
        return HandleId::from_path(path);
    }

    [[nodiscard]] auto load_untyped(std::string_view const path) const -> UntypedHandle
    {
        return UntypedHandle{ load_untracked(path) };
    }

    template <typename T>
    [[nodiscard]] auto load(std::string_view const path) const -> Handle<T>
    {
        return *load_untyped(path).typed<T>();
    }

    template <typename T>
    void update_assets(Assets<T>& assets) const
    {
        auto new_assets = [&]() -> std::vector<StoredAsset> {
            auto stored_assets = m_internal->stored_assets.write();
            if (auto const iter = stored_assets->find(type_id<T>()); iter != stored_assets->end()) {
                // TODO: should we remove the vector<> from the loaded_assets for this type `T`?
                auto new_assets = std::exchange(iter->second, std::vector<StoredAsset>{});
                return new_assets;
            }
            return {};
        }();

        for (auto& asset : new_assets) {
            assets.set_asset(HandleId{ asset.path_id }, MOV(*static_cast<T*>(asset.data.take())));
        }
    }
};