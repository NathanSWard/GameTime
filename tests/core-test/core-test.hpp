#pragma once

void asset_server_test();
void assets_test();
void asset_io_impl_test();
void handle_test();
void game_test();
void resource_test();
void system_test();

void core_test()
{
    asset_server_test();
    assets_test();
    asset_io_impl_test();
    handle_test();
    game_test();
    resource_test();
    system_test();
}