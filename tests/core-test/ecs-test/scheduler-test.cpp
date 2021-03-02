#include <ut.hpp>
#include <core/ecs/scheduler.hpp>
#include <core/ecs/resource.hpp>
#include <core/ecs/scheduler.hpp>

using namespace boost::ut;

struct AResource {};

struct StartupStage {};
struct Stage2 {};
struct Stage3 {};
struct Stage4 {};

enum class StageCount
{
    One, Two, Three, Four,
};

void scheduler_test()
{
    std::vector<StageCount> count;

    auto system1 = [&](Resource<AResource>) { count.push_back(StageCount::One); };
    auto system2 = [&](Resource<AResource>) { count.push_back(StageCount::Two); };
    auto system3 = [&](Resource<AResource>) { count.push_back(StageCount::Three); };
    auto system4 = [&](Resource<AResource>) { count.push_back(StageCount::Four); };

    "[Scheduler]"_test = [&] {
        auto r = Resources{};
        r.set_resource<AResource>();
        auto w = World{};

        auto scheduler = Scheduler{};

        scheduler.add_startup_stage<StartupStage>();
        scheduler.add_stage<Stage4>();
        scheduler.add_stage_before<Stage2, Stage4>();
        scheduler.add_stage_after<Stage3, Stage2>();

        auto const id1 = scheduler.add_system_to_startup_stage<StartupStage>(system1);
        auto const id2 = scheduler.add_system_to_stage<Stage2>(system2);
        auto const id3 = scheduler.add_system_to_stage<Stage3>(system3);
        auto const id4 = scheduler.add_system_to_stage<Stage4>(system4);

        scheduler.run_startup_stages(r, w);
        scheduler.run_stages(r, w);

        expect(count == std::vector{ StageCount::One, StageCount::Two, StageCount::Three, StageCount::Four });
    };
}