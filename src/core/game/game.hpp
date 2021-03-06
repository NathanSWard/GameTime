#pragma once

#include <core/assets/asset_server.hpp>
#include <core/game/events.hpp>
#include <debug/debug.hpp>
#include <core/ecs/resource.hpp>
#include <core/ecs/world.hpp>
#include <core/ecs/scheduler.hpp>
#include <functional>
#include <type_traits>

namespace CoreStages {

    struct Startup {};
    struct PreEvents {};
    struct Events {};
    struct PreUpdate {};
    struct Update {};
    struct PostUpdate {};

} // namespace stages

// event used to indicate the game should exit.
struct GameExit {};

struct Game
{
    World world;
    Resources resources;
    Scheduler scheduler;
    std::function<void(Game&)> runner = [](Game& g) { g.update(); };

    void update()
    {
        scheduler.run_stages(resources, world);
    }

    void run()
    {
        scheduler.run_startup_stages(resources, world);
        runner(*this);
    }
};

class GameBuilder;

template <typename P>
concept Plugin = requires(P&& plugin, GameBuilder& builder)
{
    { FWD(plugin).build(builder) };
};

class GameBuilder {
    Game m_game;
    entity_t m_curr_entity = null_entity;

public:
    GameBuilder()
    {
        add_default_stages();
        add_event<GameExit>();
    }

    GameBuilder(Game&& game) noexcept
        : m_game(MOV(game))
    {}

    // resources
    auto resources() noexcept -> Resources&
    {
        return m_game.resources;
    }

    template <typename R, typename... Args>
    auto try_add_resource(Args&&... args) -> GameBuilder&
    {
        resources().try_add_resource<R>(FWD(args)...);
        return *this;
    }

    template <typename R, typename... Args>
    auto set_resource(Args&&... args) -> GameBuilder&
    {
        resources().set_resource<R>(FWD(args)...);
        return *this;
    }

    // events
    template <typename T>
    auto add_event() -> GameBuilder&
    {
        return try_add_resource<Events<T>>()
            .add_system_to_stage<CoreStages::Events>(events_update_system<T>);
    }

    // plugins
    template <Plugin P>
    auto add_plugin(P&& p) -> GameBuilder&
    {
        FWD(p).build(*this);
        return *this;
    }

    // stages
    template <typename StageT>
    auto add_startup_stage()->GameBuilder&
    {
        m_game.scheduler.add_startup_stage<StageT>();
        return *this;
    }

    template <typename StageT>
    auto add_stage() -> GameBuilder&
    {
        m_game.scheduler.add_stage<StageT>();
        return *this;
    }

    template <typename StageT, typename StageBefore>
    auto add_stage_before() -> GameBuilder&
    {
        m_game.scheduler.add_stage_before<StageT, StageBefore>();
        return *this;
    }


    template <typename StageT, typename StageAfter>
    auto add_stage_after() -> GameBuilder&
    {
        m_game.scheduler.add_stage_after<StageT, StageAfter>();
        return *this;
    }


    auto add_default_stages() -> GameBuilder&
    {
        return add_startup_stage<CoreStages::Startup>()
            .add_stage<CoreStages::PreEvents>()
            .add_stage<CoreStages::Events>()
            .add_stage<CoreStages::PreUpdate>()
            .add_stage<CoreStages::Update>()
            .add_stage<CoreStages::PostUpdate>();
    }

    // systems
    template <typename StageTag, typename S>
    auto add_system_to_stage(S&& s) -> GameBuilder&
    {
        m_game.scheduler.add_system_to_stage<StageTag>(FWD(s));
        return *this;
    }

    template <typename StageTag, typename S>
    auto add_system_to_startup_stage(S&& s) -> GameBuilder&
    {
        m_game.scheduler.add_system_to_startup_stage<StageTag>(FWD(s));
        return *this;
    }

    template <typename S>
    auto add_system(S&& s) -> GameBuilder&
    {
        m_game.scheduler.add_system_to_stage<CoreStages::Update>(FWD(s));
        return *this;
    }

    template <typename S>
    auto add_startup_system(S&& s) -> GameBuilder&
    {
        m_game.scheduler.add_system_to_startup_stage<CoreStages::Startup>(FWD(s));
        return *this;
    }

    // components
    template <typename... Cs>
    auto prepare_components() -> GameBuilder&
    {
        (m_game.world.prepare<Cs>(), ...);
        return *this;
    }

    // assets
    template <typename T>
    auto add_asset() -> GameBuilder&
    {
        DEBUG_ASSERT(resources().contains_resource<AssetServer>());
        auto server = *(resources().get_resource<AssetServer>());
        set_resource<Assets<T>>(server->register_asset_type<T>())
            .add_system_to_stage<AssetStage::LoadAssets>(update_assets_system<T>)
            // TODO: asset_event_system
            .prepare_components<Handle<T>>();
        return *this;
    }

    template <typename T, typename... Args>
    auto add_asset_loader(Args&&... args) -> GameBuilder&
    {
        DEBUG_ASSERT(resources().contains_resource<AssetServer>());
        auto server = *(resources().get_resource<AssetServer>());
        server->add_asset_loader<T>(FWD(args)...);
        return *this;
    }

    // runner
    template <typename F>
    requires (std::is_invocable_v<F, Game&>)
    auto set_runner(F&& f) -> GameBuilder&
    {
        m_game.runner = FWD(f);
        return *this;
    }

    // build
    auto build() && -> Game
    {
        return MOV(m_game);
    }
};