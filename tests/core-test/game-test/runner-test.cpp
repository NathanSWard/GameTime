#include "ut.hpp"
#include <core/game/runner.hpp>

using namespace boost::ut;

void game_exit_system(Resource<Events<GameExit>> game_exit_events)
{
    game_exit_events->send(GameExit{});
}

void runner_test()
{
    auto builder = GameBuilder();
    builder.add_plugin(SchedulerRunnerPlugin{});
    builder.add_system(game_exit_system);

    auto game = MOV(builder).build();
    game.run();

    expect(true); // .run() should not infinite loop
}