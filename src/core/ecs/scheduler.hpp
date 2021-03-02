#pragma once

#include "resource.hpp"
#include "system.hpp"
#include "world.hpp"

#include <debug/debug.hpp>
#include <util/common.hpp>
#include <util/rng.hpp>

#include <algorithm>
#include <unordered_map>
#include <vector>

struct StageId 
{
    type_id_t id{};

    template <typename StageTag>
    constexpr static auto create() -> StageId
    {
        return StageId{ .id = type_id<StageTag>() };
    }

    constexpr auto operator==(StageId const& rhs) const noexcept -> bool
    {
        return id == rhs.id;
    }

    constexpr auto operator!=(StageId const& rhs) const noexcept -> bool
    {
        return !(*this == rhs);
    }
};

class Stage 
{
protected:
    std::vector<System> m_systems;
    StageId m_id;

    Stage(StageId const id) noexcept : m_id(id) {}

public:
    template <typename StageTag>
    constexpr static auto create() -> Stage
    {
        return Stage(StageId::create<StageTag>());
    }

    constexpr Stage(Stage&&) noexcept = default;
    constexpr Stage& operator=(Stage&&) noexcept = default;

    constexpr auto id() const noexcept { return m_id; }

    template <typename S>
    void add_system(S&& s)
    {
        m_systems.emplace_back(FWD(s));
    }

    void run(Resources& resources, World& world)
    {
        for (auto& system : m_systems) {
            if (system.should_run()) {
                system.run(resources, world);
            }
        }
    }
};

class Scheduler
{
    std::vector<std::unique_ptr<Stage>> m_startup_stages;
    std::vector<std::unique_ptr<Stage>> m_stages;

    static constexpr auto extract_stage_id(std::unique_ptr<Stage> const& stage) noexcept -> StageId
    {
        return stage->id();
    }

    template <typename StageTag, typename F>
    static auto add_system_to_stage_impl(std::vector<std::unique_ptr<Stage>>& stages, F&& f) -> SystemId
    {
        auto const iter = std::ranges::find(std::as_const(stages), StageId::create<StageTag>(), extract_stage_id);
        DEBUG_ASSERT(iter != stages.end());

        auto system = System::create(FWD(f));
        auto const id = system.id();
        (*iter)->add_system(MOV(system));

        return id;
    }

    template <typename NewStage>
    static auto add_stage_impl(std::vector<std::unique_ptr<Stage>>& stages) -> StageId
    {
        DEBUG_ASSERT(std::ranges::find(std::as_const(stages), StageId::create<NewStage>(), extract_stage_id) == stages.end());
        return stages.emplace_back(std::make_unique<Stage>(Stage::create<NewStage>()))->id();
    }

    template <typename NewStage, typename StageBefore>
    static auto add_stage_before_impl(std::vector<std::unique_ptr<Stage>>& stages) -> StageId
    {
        auto const iter = std::ranges::find(std::as_const(stages), StageId::create<StageBefore>(), extract_stage_id);
        DEBUG_ASSERT(iter != stages.cend());
        return (*stages.insert(iter, std::make_unique<Stage>(Stage::create<NewStage>())))->id();
    }

    template <typename NewStage, typename StageAfter>
    static auto add_stage_after_impl(std::vector<std::unique_ptr<Stage>>& stages) -> StageId
    {
        auto const iter = std::ranges::find(std::as_const(stages), StageId::create<StageAfter>(), extract_stage_id);
        DEBUG_ASSERT(iter != stages.cend());
        return (*stages.insert(std::next(iter), std::make_unique<Stage>(Stage::create<NewStage>())))->id();
    }

public:
    // stages
    template <typename StageTag, typename F>
    auto add_system_to_stage(F&& f)
    {
        return add_system_to_stage_impl<StageTag>(m_stages, FWD(f));
    }

    template <typename StageTag>
    auto add_stage()
    {
        return add_stage_impl<StageTag>(m_stages);
    }

    template <typename StageTag, typename StageBefore>
    auto add_stage_before()
    {
        return add_stage_before_impl<StageTag, StageBefore>(m_stages);
    }

    template <typename StageTag, typename StageAfter>
    auto add_stage_after()
    {
        return add_stage_after_impl<StageTag, StageAfter>(m_stages);
    }

    // startup_stage
    template <typename StageTag, typename F>
    auto add_system_to_startup_stage(F&& f)
    {
        return add_system_to_stage_impl<StageTag>(m_startup_stages, FWD(f));
    }

    template <typename StageTag>
    auto add_startup_stage()
    {
        return add_stage_impl<StageTag>(m_startup_stages);
    }

    template <typename StageTag, typename StageBefore>
    auto add_startup_stage_before()
    {
        return add_stage_before_impl<StageTag, StageBefore>(m_startup_stages);
    }

    template <typename StageTag, typename StageAfter>
    auto add_startup_stage_after()
    {
        return add_stage_after_impl<StageTag, StageAfter>(m_startup_stages);
    }

    // running stages
    void run_startup_stages(Resources& resources, World& world)
    {
        for (auto& stage : m_startup_stages) {
            stage->run(resources, world);
        }
    }

    void run_stages(Resources& resources, World& world)
    {
        for (auto& stage : m_stages) {
            stage->run(resources, world);
        }
    }
};