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
    void build(Game& game)
    {
        if (!game.resources().get_resource<AssetServer>().has_value()) {
            auto const settings = [&game] {
                if (auto const settings = game.resources().get_resource<AssetServerSettings>(); settings) {
                    return *MOV(settings);
                }
                else {
                    return game.resources().add_resource<AssetServerSettings>();
                }
            }();

            auto asset_io = std::make_unique<FileAssetIo>(settings->asset_folder);
            game.add_resource<AssetServer>(MOV(asset_io), TaskPool{});
            game.prepare_components<UntypedHandle>();
        }
    }
};