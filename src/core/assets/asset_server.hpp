#pragma once

#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

#include <debug/debug.hpp>
#include <util/common.hpp>
#include <util/containers/hash.hpp>
#include <util/sync/rwlock.hpp>
#include <core/ecs/resource.hpp>
#include <core/task/task_pool.hpp>

#include "assets.hpp"
#include "asset_io/asset_io.hpp"
#include "handle.hpp"
#include "loader.hpp"
#include "ref_change.hpp"

namespace AssetStage {

    struct LoadAssets {};
    struct AssetEvents {};

} // namespace AssetStage


enum class LoadState
{
    NotLoaded,
    Loading,
    Loaded,
    Failed,
};

struct AssetInfo
{
    std::filesystem::path   path;
    LoadState               load_state = LoadState::NotLoaded;
    tl::optional<type_id_t> type_id;
    std::size_t             version = 0;
};

namespace as_detail {
    struct StoredAsset
    {
        void_ptr data;
        AssetPathId path_id;
    };

    struct AssetRefCounter
    {
        RefChangeChannel channel;
        RwLock<HashMap<HandleId, std::size_t>> ref_counts;
    };

    struct AssetServerInternal
    {
        AssetServerInternal(std::unique_ptr<AssetIo> asset_io, TaskPool taskpool)
            : asset_io(MOV(asset_io))
            , ref_counter(AssetRefCounter{
                .channel = RefChangeChannel::create(),
                })
            , task_pool(MOV(taskpool))
        {}

        AssetServerInternal(AssetServerInternal&&) noexcept = default;
        AssetServerInternal& operator=(AssetServerInternal&&) noexcept = default;

        TaskPool task_pool;
        std::unique_ptr<AssetIo> asset_io;
        AssetRefCounter ref_counter;
        RwLock<std::vector<std::shared_ptr<AssetLoader>>> loaders;
        RwLock<HashMap<std::string, std::size_t, hash::string_hash, hash::string_equal>> extension_to_loader_index;
        RwLock<HashMap<AssetPathId, AssetInfo>> asset_info;
        RwLock<HashMap<type_id_t, std::vector<StoredAsset>>> stored_assets;
        RwLock<HashMap<type_id_t, std::vector<HandleId>>> assets_to_free;
    };
}

class AssetServer
{
    std::shared_ptr<as_detail::AssetServerInternal> m_internal;
    using StoredAsset = as_detail::StoredAsset;

public:

    enum Error
    {
        MissingAssetLoader,
        IncorrectHandleType,
        AssetLoaderError,
        AssetIoError,
        AssetFolderNotADirectory,
    };

    template <typename T>
    using AssetServerResult = tl::expected<T, Error>;

    template <IsAssetIo AIo>
    AssetServer(std::unique_ptr<AIo> asset_io, TaskPool taskpool) noexcept
        : m_internal(std::make_shared<as_detail::AssetServerInternal>(MOV(asset_io), MOV(taskpool)))
    {
        DEBUG_ASSERT(m_internal->asset_io != nullptr, "AssetServer asset_io cannot be null");
    }

    AssetServer(AssetServer&&) noexcept = default;
    AssetServer& operator=(AssetServer&&) noexcept = default;
    AssetServer(AssetServer const&) noexcept = default;
    AssetServer& operator=(AssetServer const&) noexcept = default;

    template <typename T>
    [[nodiscard]] auto get_handle(HandleId const id) const -> Handle<T>
    {
        auto sender = m_internal->ref_counter.channel.sender;
        return Handle<T>::strong(id, MOV(sender));
    }

    [[nodiscard]] auto get_untyped_handle(HandleId const id) const -> UntypedHandle
    {
        auto sender = m_internal->ref_counter.channel.sender;
        return UntypedHandle::strong(id, MOV(sender));
    }

    template <typename T>
    [[nodiscard]] auto register_asset_type() const -> Assets<T>
    {
        auto stored_assets = m_internal->stored_assets.write();
        stored_assets->emplace(std::piecewise_construct,
            std::forward_as_tuple(type_id<T>()),
            std::forward_as_tuple()
        );
        return Assets<T>(m_internal->ref_counter.channel.sender);
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

    [[nodiscard]] auto get_asset_loader_from_path(std::filesystem::path const& path) const -> tl::optional<std::shared_ptr<AssetLoader>>
    {
        auto filename = path.filename().string();
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

    [[nodiscard]] auto get_load_state(HandleId const id) const -> LoadState
    {
        if (!id.m_is_path_id) {
            return LoadState::NotLoaded;
        }

        auto asset_info = m_internal->asset_info.read();
        if (auto const iter = asset_info->find(id.m_path_id); iter != asset_info->end()) {
            return iter->second.load_state;
        }
        return LoadState::NotLoaded;
    }

    // TODO: Make async??
    [[nodiscard]] auto load_sync(std::filesystem::path const& path) const -> AssetServerResult<AssetPathId>
    {
        auto loader = get_asset_loader_from_path(path);
        if (!loader) {
            return tl::make_unexpected(Error::MissingAssetLoader);
        }

        auto const path_id = HandleId::from_path(path).m_path_id;

        auto const version_opt = [&]() -> tl::optional<std::size_t> {
            auto asset_info = m_internal->asset_info.write();
            
            auto const [iter, inserted] = 
                asset_info->emplace(std::piecewise_construct,
                    std::forward_as_tuple(path_id), 
                    std::forward_as_tuple( // AssetPathInfo
                        path, // path
                        LoadState::NotLoaded, // load_state
                        tl::nullopt, // type_id
                        0 // version
                    ));

            // TODO: Possibly give the option to force the asset to reload?
            //       Also, if the asset is already loaded, should we remove the info?
            // When this is updated: I'll have to update the `update_assets` to ensure versions match
            if (!inserted) {// asset already exists
                return {};
            }

            auto& info = iter->second;
            info.load_state = LoadState::Loading;
            info.version += 1;
            return info.version;
        }();

        if (!version_opt) {
            return path_id;
        }

        auto const version = *version_opt;

        auto set_load_state = [&](LoadState const state) {
            auto asset_info = m_internal->asset_info.write();
            auto info = asset_info->find(path_id);
            DEBUG_ASSERT(info != asset_info->end(), "Unable to find path_id: {}", path_id);
            info->second.load_state = state;
        };

        // load the asset file's bytes
        auto bytes = m_internal->asset_io->load_path(path)();
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
            DEBUG_ASSERT(info != asset_info->end(), "Unable to find path_id: {}", path_id);

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

    [[nodiscard]] auto load_untracked(std::filesystem::path const& path) const -> HandleId
    {
        // TODO: Revamp TaskPool to return a `future` like object
         m_internal->task_pool.execute([server = *this, path = path]{
            if (auto const result = server.load_sync(path); !result) {
                spdlog::error("AssetServer failed to load assets: '{}'", path.string());
            }
            });
        return HandleId::from_path(path);
    }

    [[nodiscard]] auto load_untyped(std::filesystem::path const& path) const -> UntypedHandle
    {
        auto const id = load_untracked(path);
        return get_untyped_handle(id);
    }

    template <typename T>
    [[nodiscard]] auto load(std::filesystem::path const& path) const -> Handle<T>
    {
        return load_untyped(path).typed<T>();
    }

    [[nodiscard]] auto load_folder(std::filesystem::path const& dir) const -> tl::expected<std::vector<UntypedHandle>, Error>
    {
        if (!m_internal->asset_io->is_directory(dir)) {
            return tl::make_unexpected(AssetFolderNotADirectory);
        }

        auto handles = std::vector<UntypedHandle>{};
        auto iter = m_internal->asset_io->read_directory(dir);
        if (!iter) {
            return tl::make_unexpected(AssetIoError);
        }

        for (auto const& child_path : *iter) {
            if (m_internal->asset_io->is_directory(child_path.path())) {
                auto inner_handles = load_folder(child_path);
                if (!inner_handles) {
                    return tl::make_unexpected(inner_handles.error());
                }
                handles.insert(
                    handles.end(),
                    std::make_move_iterator(inner_handles->begin()),
                    std::make_move_iterator(inner_handles->end())
                );
            }
            else {
                if (!get_asset_loader_from_path(child_path.path().string()).has_value()) {
                    continue;
                }
                handles.emplace_back(load_untyped(child_path));
            }
        }

        return handles;
    }

    void update_asset_ref_count() const
    {
        auto potential_frees = std::vector<HandleId>();

        auto& receiver = m_internal->ref_counter.channel.receiver;
        auto ref_counts = m_internal->ref_counter.ref_counts.write();

        for (;;) {
            auto rc = receiver.recv();
            if (!rc.has_value()) {
                break;
            }

            switch (rc->type) {
                case RefChange::Increment:
                    (*ref_counts)[rc->id] += 1;
                    break;
                case RefChange::Decrement: {
                    auto [iter, inserted] = ref_counts->try_emplace(rc->id, 0);
                    UNUSED(inserted);

                    auto& value = iter->second;
                    value -= 1;
                    if (value == 0) {
                        potential_frees.push_back(rc->id);
                        ref_counts->erase(iter);
                    }
                } break;
                default: // unreachable
                    break;
                }
        }

        if (!potential_frees.empty()) {
            
            auto assets_to_free = m_internal->assets_to_free.write();
            auto asset_info = m_internal->asset_info.write();

            for (auto const& id : potential_frees) {
                DEBUG_ASSERT(!ref_counts->contains(id), "AssetServer trying to free a valid asset handle.");
                
                auto const tid = [&]() -> tl::optional<type_id_t> {
                    // get type_id_t and possilby erase from the `asset_info`.
                    if (id.m_is_path_id) {
                        if (auto const iter = asset_info->find(id.m_path_id); iter != asset_info->end()) {
                            auto const tid = iter->second.type_id;
                            asset_info->erase(iter);
                            return tid;
                        }
                        return {};
                    }
                    else { // is uuid 
                        return id.m_uid.type_id;
                    }
                }();

                if (tid.has_value()) {
                    (*assets_to_free)[*tid].push_back(id);
                }
            }
        }
    }

    template <typename T>
    void update_assets(Assets<T>& assets) const
    {
        // add all newly created assets
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

        // remove unused assets
        {
            // First check if it even worth acquiring the write lock on the `assets_to_free`.
            auto assets_to_free = m_internal->assets_to_free.read();
            if (auto const iter = assets_to_free->find(type_id<T>()); iter == assets_to_free->end()) {
                return;
            }
            else if (iter->second.empty()){
                return;
            }
        }

        auto handle_ids = [&] {
            auto assets_to_free = m_internal->assets_to_free.write();
            auto iter = assets_to_free->find(type_id<T>());
            DEBUG_ASSERT(iter != assets_to_free->end(), "AssetServer should have assets to free for: Assets<{}>", type_name<T>());

            // TODO: should we remove the vector<> from the assets_to_free for this type `T`?
            auto handle_ids = std::vector<HandleId>();
            handle_ids.swap(iter->second);
            return handle_ids;
        }();

        for (auto const& id : handle_ids) {
            assets.remove_asset(id);
        }
    }
};

void update_asset_ref_count_system(Resource<AssetServer const> server)
{
    server->update_asset_ref_count();
}

template <typename T>
void update_assets_system(Resource<AssetServer const> server, Resource<Assets<T>> assets)
{
    server->update_assets(*assets);
}