#include "ut.hpp"
#include "core/ecs/system.hpp"
#include "core/ecs/system_scheduler.hpp"
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
}

namespace execute {
    int global_count = 0;

    void only_query(Query<With<int>>) { ++global_count; }
    void only_resource(Resource<int>) { ++global_count; }
    void query_resource(Query<With<int>>, Resource<int>) { ++global_count; }
}

void system_test()
{
    "[Create Systems: Free Function]"_test = [] {
        auto system1 = System::create(funcs::only_query, system_id_t{ 1 });
        auto system2 = System::create(funcs::only_resource, system_id_t{ 2 });
        auto system3 = System::create(funcs::query_and_resources, system_id_t{ 3 });
        auto system4 = System::create(funcs::query_group, system_id_t{ 4 });
        auto system5 = System::create(funcs::multiple_queries, system_id_t{ 5 });
        auto system6 = System::create(funcs::multiple_resources, system_id_t{ 6 });
        auto system7 = System::create(funcs::multiple_queries_and_resources, system_id_t{ 7 });
        auto system8 = System::create(funcs::commands, system_id_t{ 8 });
        auto system9 = System::create(funcs::comands_and_other, system_id_t{ 9 });
        auto system10 = System::create(funcs::local, system_id_t{ 10 });
        auto system11 = System::create(funcs::local_and_other, system_id_t{ 11 });
    };

    "[Create Systems: Lambda]"_test = [] {
        auto func = [](Query<With<int>>, Resource<char>) {};
        auto system = System::create(func, system_id_t{ 1 });
    };

    "[Execute System: Local Resource]"_test = [] {
        auto r = Resources{};
        auto w = World{};

        auto increment_local = [](Local<int> l) { ++(*l); };
        auto system1 = System::create(increment_local, system_id_t{ 1 });
        auto system2 = System::create(increment_local, system_id_t{ 2 });

        auto const local1 = r.local().add_local_resource<int>(system_id_t{ 1 }, 0);
        auto const local2 = r.local().add_local_resource<int>(system_id_t{ 2 }, 99);

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
        r.add_resource<char>('a'); // this should not affect the systems

        auto w = World{};

        auto system1 = System::create(execute::only_query, system_id_t{ 1 });
        auto system2 = System::create(execute::only_resource, system_id_t{ 2 });
        auto system3 = System::create(execute::query_resource, system_id_t{ 3 });

        should("execute system with only query") = [&] {
            execute::global_count = 0;
            system1.run(r, w);
            expect(execute::global_count == 1);
        };

        should("lacking resource sholud not execute sytem") = [&] {
            execute::global_count = 0;
            system2.run(r, w);
            system3.run(r, w);
            expect(execute::global_count == 0);
        };

        // add required resource
        r.add_resource<int>(32);

        should("systems with resources should execute") = [&] {
            execute::global_count = 0;
            system1.run(r, w);
            system2.run(r, w);
            system3.run(r, w);
            expect(execute::global_count == 3);
        };

        // remove resource
        r.remove_resource<int>();
        should("after removing resource, should not execute sytem") = [&] {
            execute::global_count = 0;
            system2.run(r, w);
            system3.run(r, w);
            expect(execute::global_count == 0);
        };
    };

    "[System Scheduler]"_test = [] {
        auto r = Resources{};
        r.add_resource<int>(42);
        auto w = World{};

        auto scheduler = Scheduler{};

        auto const id1 = scheduler.add_system(execute::only_query);
        auto const id2 = scheduler.add_system(execute::only_resource);
        auto const id3 = scheduler.add_system(execute::query_resource);

        should("have run systems") = [&] {
            execute::global_count = 0;
            scheduler.run_systems(r, w);
            expect(execute::global_count == 3);
        };

        expect(scheduler.remove_system(id1) == true);
        expect(scheduler.remove_system(id2) == true);

        should("unable to delete an invaild system `id`") = [&] {
            expect(scheduler.remove_system(id1) == false);
            expect(scheduler.remove_system(id2) == false);
        };

        should("run single remaining system") = [&] {
            execute::global_count = 0;
            scheduler.run_systems(r, w);
            expect(execute::global_count == 1);
        };

        expect(scheduler.remove_system(id3) == true);
        should("run nothing: no remaining system") = [&] {
            execute::global_count = 0;
            scheduler.run_systems(r, w);
            expect(execute::global_count == 0);
        };
    };

    "[System Scheduler: Validate Resources]"_test = [] {
        auto r = Resources{};
        r.add_resource<int>(0);
        auto w = World{};
        auto scheduler = Scheduler{};

        auto res = r.get_resource<int const>();
        auto increment_resource = [](Resource<int> res) {
            ++(*res);
        };

        should("add system") = [&] {
            scheduler.add_system(increment_resource);
            scheduler.run_systems(r, w);
            expect(**res == 1);
        };

        should("add additional systems") = [&] {
            scheduler.add_system(increment_resource);
            scheduler.add_system(increment_resource);
            scheduler.run_systems(r, w);
            expect(**res == 4);
        };
    };

    "[Commands]"_test = [] {
        auto r = Resources{};
        auto w = World{};
        auto scheduler = Scheduler{};
    
        auto add_entity_and_resource = [](Commands cmds) {
            auto e = cmds.spawn();
            cmds.add_component<int>(e, 42);
            cmds.add_resource<int>(42);
        };

        auto system = scheduler.add_system(add_entity_and_resource);
        scheduler.run_systems(r, w);

        expect(w.size() == 1);
        expect(w.view<int>().size() == 1);
        w.view<int>().each([](int const i) { expect(i == 42); });

        auto const res = r.get_resource<int const>();
        expect((res.has_value()) >> fatal);
        expect(**res == 42);
    };
}