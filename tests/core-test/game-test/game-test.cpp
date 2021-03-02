#include "ut.hpp"
#include <core/game/game.hpp>

using namespace boost::ut;

namespace game_test_ns {

    int ran_startup = 0;

}

void startup_system()
{
    ++game_test_ns::ran_startup;
}

struct test_plugin
{
    inline static int plugin_ran = 0;

    void build(GameBuilder& builder)
    {
        ++plugin_ran;
        builder.add_startup_system(startup_system);
    }
};

void game_test()
{
    GameBuilder builder;
    builder.add_plugin(test_plugin{});
    
    auto game = MOV(builder).build();
    game.run();

    expect(test_plugin::plugin_ran == 1);
    expect(game_test_ns::ran_startup == 1);
}