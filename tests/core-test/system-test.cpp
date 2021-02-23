#include "../ut/ut.hpp"
#include "core/system.hpp"
#include "core/system_scheduler.hpp"
#include "core/resource.hpp"

using namespace boost::ut;

namespace funcs {
    void only_query(Query<With<int>>) {}
    void only_resource(Resource<int>) {}
    void query_and_resources(Query<With<int>>, Resource<int>) {}
    void query_group(Query<With<int>, Without<char>, Group>) {}
    void multiple_queries(Query<With<int>>, Query<With<char>>) {}
    void multiple_resources(Resource<int>, Resource<char>) {}
    void multiple_queries_and_resources(Resource<int>, Resource<char>, Query<With<int>>, Query<With<char>>) {}
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
        auto system1 = System::create(funcs::only_query);
        auto system2 = System::create(funcs::only_resource);
        auto system3 = System::create(funcs::query_and_resources);
        auto system4 = System::create(funcs::query_group);
        auto system5 = System::create(funcs::multiple_queries);
        auto system6 = System::create(funcs::multiple_resources);
        auto system7 = System::create(funcs::multiple_queries_and_resources);
    };

    "[Create Systems: Lambda]"_test = [] {
        auto func = [](Query<With<int>>, Resource<char>) {};
        auto system = System::create(func);
    };

    "[Execute System]"_test = [] {
        auto r = Resources{};
        r.add_resource<char>('a'); // this should not affect the systems

        auto w = World{};

        auto system1 = System::create(execute::only_query);
        auto system2 = System::create(execute::only_resource);
        auto system3 = System::create(execute::query_resource);

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
}