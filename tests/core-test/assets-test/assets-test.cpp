#include "ut.hpp"

#include <core/assets/assets.hpp>
#include <core/assets/handle.hpp>

using namespace boost::ut;

void assets_test()
{
    "[Assest]"_test = [] {
        auto [send, recv] = mpmc_channel<RefChange>();
        auto as = Assets<int>(MOV(send));

        auto handle1 = as.add_asset(1);
        auto handle2 = as.add_asset(2);

        expect(handle1.id() != handle2.id());

        should("have same id") = [&] {
            auto handle1_copy = as.get_handle(handle1.id());
            expect(handle1.id() == handle1_copy.id());
        };

        should("contain assets") = [&] {
            expect(as.contains_asset(handle1));
            expect(as.contains_asset(handle2));
        };

        should("get handle's value") = [&] {
            auto value1 = as.get_asset(handle1);
            auto value2 = as.get_mut_asset_untracked(handle2);

            static_assert(std::is_same_v<decltype(*value1), int const&>);
            static_assert(std::is_same_v<decltype(*value2), int&>);

            expect((value1.has_value() && value2.has_value()) >> fatal);
            expect(*value1 == 1 && *value2 == 2);
        };

        as.set_asset(handle1, 99);
        should("have updated asset") = [&] {
            auto value = as.get_asset(handle1);

            expect((value.has_value()) >> fatal);
            expect(*value == 99);
            
        };

        should("remove assets") = [&] {
            auto value1 = as.remove_asset(handle1);
            auto value2 = as.remove_asset(handle2);

            expect((value1.has_value()) >> fatal);
            expect((value2.has_value()) >> fatal);

            expect(*value1 == 99);
            expect(*value2 == 2);

            should("not have assets") = [&] {
                expect(!as.contains_asset(handle1));
                expect(!as.contains_asset(handle2));

                expect(!as.get_asset(handle1));
                expect(!as.get_asset(handle2));
            };
        };
    };

    "[AssetEvent]"_test = [] {
        auto [send, recv] = mpmc_channel<RefChange>();
        auto as = Assets<int>(MOV(send));

        Events<AssetEvent<int>> events;
        auto reader = events.get_reader();

        as.update_events(events);
        expect(reader.iter(events).size() == 0);

        auto check_event = [&](auto type) {
            as.update_events(events);
            auto iter = reader.iter(events);
            expect((iter.size() == 1) >> fatal);
            auto const& event = *iter.begin();
            expect(event.type == type);
        };

        auto handle = as.add_asset(42);
        check_event(AssetEvent<int>::Created);

        as.set_asset(handle.id(), 2);
        check_event(AssetEvent<int>::Modified);

        as.remove_asset(handle.id());
        check_event(AssetEvent<int>::Removed);
    };
}