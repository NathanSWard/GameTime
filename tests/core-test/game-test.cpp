#include "ut.hpp"
#include <core/game.hpp>

using namespace boost::ut;

int ran_startup = 0;

void startup_system()
{
    ++ran_startup;
}

void quit_game(Resource<Quit> quit)
{
    (*quit).quit = true;
}

struct test_plugin
{
    inline static int plugin_ran = 0;

    void build(Game& game)
    {
        ++plugin_ran;
        game.add_startup_system(startup_system);
        game.add_system(quit_game);
    }
};

void game_test()
{
    Game g{};
    g.add_plugin(test_plugin{});
    g.run();
    expect(true); // ensure game will not infinite loop
    expect(test_plugin::plugin_ran == 1);
    expect(ran_startup == 1);
}