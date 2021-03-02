#pragma once

#include <core/assets/asset_server.hpp>
#include <core/assets/asset_io/asset_io_impl.hpp>
#include <core/game.hpp>
#include <core/task/task_pool.hpp>

struct AssetServerSettings
{
    std::string asset_folder = "assets";
};

struct AssetPlugin
{
    void build(GameBuilder& builder)
    {
        if (!builder.resources().get_resource<AssetServer>().has_value()) {
            auto const settings = [&builder] {
                if (auto const settings = builder.resources().get_resource<AssetServerSettings>(); settings) {
                    return *MOV(settings);
                }
                else {
                    return builder.resources().add_resource<AssetServerSettings>();
                }
            }();

            auto asset_io = std::make_unique<FileAssetIo>(settings->asset_folder);
            builder
                .add_resource<AssetServer>(MOV(asset_io), TaskPool{})
                .prepare_components<UntypedHandle>();
        }

        builder
            .add_stage_before<AssetStage::LoadAssets, CoreStages::PreUpdate>();
        // TODO: free_unused_assets_system
    }
};