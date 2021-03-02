#pragma once

#include "game.hpp"
#include <chrono>
#include <thread>
#include <tl/optional.hpp>

struct RunMode
{
    enum Type
    {
        Loop,
        Once,
    } type = Loop;
    tl::optional<std::chrono::duration<float>> wait;
};

struct SchedulerRunnerSettings
{
    RunMode run_mode;

    constexpr static auto run_once() noexcept -> SchedulerRunnerSettings
    {
        return SchedulerRunnerSettings{
            .run_mode = RunMode {
                .type = RunMode::Once,
                .wait = tl::nullopt,
            },
        };
    }

    constexpr static auto run_loop(std::chrono::duration<float> const wait) 
        noexcept -> SchedulerRunnerSettings
    {
        return SchedulerRunnerSettings{
            .run_mode = RunMode {
                .type = RunMode::Loop,
                .wait = wait,
            },
        };
    }
};

struct SchedulerRunnerPlugin
{
    void build(GameBuilder& builder)
    {
        using clock_t = std::chrono::high_resolution_clock;
        using dur_t = std::chrono::duration<float>;

        auto settings = *(builder
            .resources()
            .try_add_resource<SchedulerRunnerSettings>());

        auto runner = [settings = MOV(settings)](Game& game) {
            switch (settings.run_mode.type) {
            case RunMode::Once:
                game.update();
                break;
            case RunMode::Loop: {
                auto game_exit_event_reader = ManualEventReader<GameExit>();

                auto tick = [&]() -> tl::expected<tl::optional<dur_t>, GameExit> {
                    auto check_for_app_exit = [&]() -> bool {
                        auto events = game.resources.get_resource<Events<GameExit>>();
                        if (!events) {
                            return false;
                        }

                        if (game_exit_event_reader.iter(**events).size() > 0) {
                            return true;
                        }

                        return false;
                    };

                    auto const start_time = clock_t::now();

                    if (check_for_app_exit()) {
                        return tl::make_unexpected(GameExit{});
                    }

                    game.update();

                    if (check_for_app_exit()) {
                        return tl::make_unexpected(GameExit{});
                    }

                    auto const end_time = clock_t::now();

                    auto const& wait = settings.run_mode.wait;
                    if (wait) {
                        auto const execution_time = std::chrono::duration_cast<dur_t>(end_time - start_time);
                        if (execution_time < *wait) {
                            return tl::make_optional<dur_t>(*wait - execution_time);
                        }
                    }

                    return tl::optional<dur_t>();
                };

                for (;;) {
                    auto const delay = tick();
                    if (!delay) { // AppExit Event Received
                        return;
                    }
                    if (delay->has_value()) {
                        std::this_thread::sleep_for(**delay);
                    }
                }
            }
                break;
            default: // unreachable
                break;
            };
        };

        builder.set_runner(runner);
    }
};