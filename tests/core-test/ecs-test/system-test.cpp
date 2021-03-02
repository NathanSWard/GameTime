#include "ut.hpp"
#include "core/ecs/system.hpp"
#include "core/ecs/scheduler.hpp"
#include "core/ecs/resource.hpp"

using namespace boost::ut;

namespace funcs {
    void only_query(Query<With<int>>) {}
    void only_resource(Resource<int>) {}
    void query_and_resources(Query<With<int>>, Resource<int>) {}
    void query_group(Query<With<int>, Without<char>, Group>) {}
    void multiple_queries(Query<With<int>>, Query<With<char>>) {}
    void multiple_resources(Resource<int>, Resource<char>) {}
    void multiple_queries_and_resources(Resource<int>, Resource<char>, Query<With<int>>, Query<With<char>>) {}
    void commands(Commands) {}
    void comands_and_other(Commands, Query<With<int>>, Resource<char>) {}
    void local(Local<int>) {}
    void local_and_other(Local<int>, Commands, Query<With<int>>, Resource<char>) {}
    void event_reader(EventReader<int>) {}
}

namespace system_test_execute {
    int global_count = 0;

    void only_query(Query<With<int>>) { ++global_count; }
    void only_resource(Resource<int>) { ++global_count; }
    void query_resource(Query<With<int>>, Resource<int>) { ++global_count; }
}

void system_test()
{
    "[Create Systems: Free Function]"_test = [] {
        auto system1 = System::create(funcs::only_query);
        auto system2 = System::create(funcs::only_resource);
        auto system3 = System::create(funcs::query_and_resources);
        auto system4 = System::create(funcs::query_group);
        auto system5 = System::create(funcs::multiple_queries);
        auto system6 = System::create(funcs::multiple_resources);
        auto system7 = System::create(funcs::multiple_queries_and_resources);
        auto system8 = System::create(funcs::commands);
        auto system9 = System::create(funcs::comands_and_other);
        auto system10 = System::create(funcs::local);
        auto system11 = System::create(funcs::local_and_other);
        auto system12 = System::create(funcs::event_reader);
    };

    "[Create Systems: Lambda]"_test = [] {
        auto func = [](Query<With<int>>, Resource<char>) {};
        auto system = System::create(func);
    };

    "[Execute System: Local Resource]"_test = [] {
        auto r = Resources{};
        auto w = World{};

        auto increment_local = [](Local<int> l) { ++(*l); };
        auto increment_local2 = [](Local<int> l) { ++(*l); };

        auto system1 = System::create(increment_local);
        auto system2 = System::create(increment_local2);

        auto const local1 = r.local().set_local_resource<int>(system1.id(), 0);
        auto const local2 = r.local().set_local_resource<int>(system2.id(), 99);

        expect(*local1 == 0);
        expect(*local2 == 99);

        system1.run(r, w);
        system2.run(r, w);

        should("not affect other systems's locals") = [&] {
            expect(*local1 == 1);
            expect(*local2 == 100);
        };
    };

    "[Execute System]"_test = [] {
        auto r = Resources{};
        r.set_resource<char>('a'); // this should not affect the systems

        auto w = World{};

        auto system1 = System::create(system_test_execute::only_query);
        auto system2 = System::create(system_test_execute::only_resource);
        auto system3 = System::create(system_test_execute::query_resource);

        should("execute system with only query") = [&] {
            system_test_execute::global_count = 0;
            system1.run(r, w);
            expect(system_test_execute::global_count == 1);
        };

        should("lacking resource sholud not execute sytem") = [&] {
            system_test_execute::global_count = 0;
            system2.run(r, w);
            system3.run(r, w);
            expect(system_test_execute::global_count == 0);
        };

        // add required resource
        r.set_resource<int>(32);

        should("systems with resources should execute") = [&] {
            system_test_execute::global_count = 0;
            system1.run(r, w);
            system2.run(r, w);
            system3.run(r, w);
            expect(system_test_execute::global_count == 3);
        };

        // remove resource
        r.remove_resource<int>();
        should("after removing resource, should not execute sytem") = [&] {
            system_test_execute::global_count = 0;
            system2.run(r, w);
            system3.run(r, w);
            expect(system_test_execute::global_count == 0);
        };
    };

    "[Commands]"_test = [] {
        auto r = Resources{};
        auto w = World{};
        auto scheduler = Scheduler{};

        struct StageT {};
        scheduler.add_stage<StageT>();
    
        auto add_entity_and_resource = [](Commands cmds) {
            auto e = cmds.spawn();
            cmds.add_component<int>(e, 42);
            cmds.set_resource<int>(42);
        };

        auto system = scheduler.add_system_to_stage<StageT>(add_entity_and_resource);
        scheduler.run_stages(r, w);

        expect(w.size() == 1);
        expect(w.view<int>().size() == 1);
        w.view<int>().each([](int const i) { expect(i == 42); });

        auto const res = r.get_resource<int const>();
        expect((res.has_value()) >> fatal);
        expect(**res == 42);
    };
}