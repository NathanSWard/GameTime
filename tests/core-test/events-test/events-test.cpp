#include <ut.hpp>
#include <core/events/events.hpp>

using namespace boost::ut;

template <typename T>
auto get_events(Events<T> const& events, ManualEventReader<T>& reader)
{
    std::vector<T> vec;
    for (auto const& value : reader.iter(events)) {
        vec.push_back(value);
    }
    return vec;
}

template <typename T>
auto get_events(EventReader<T>& reader)
{
    std::vector<T> vec;
    for (auto const& value : reader.iter()) {
        vec.push_back(value);
    }
    return vec;
}

void events_test()
{
    "[Events]"_test = [] {
        auto events = Events<int>();

        auto const event0 = 0;
        auto const event1 = 1;
        auto const event2 = 2;

        auto reader_missed = events.get_reader();
        auto reader_a = events.get_reader();

        events.send(event0);

        expect(get_events(events, reader_a) == std::vector{ event0 });
        expect(get_events(events, reader_a) == std::vector<int>{});

        auto reader_b = events.get_reader();

        expect(get_events(events, reader_b) == std::vector{ event0 });
        expect(get_events(events, reader_b) == std::vector<int>{});

        events.send(event1);

        auto reader_c = events.get_reader();

        expect(get_events(events, reader_c) == std::vector{ event0, event1 });
        expect(get_events(events, reader_c) == std::vector<int>{});

        expect(get_events(events, reader_a) == std::vector{ event1 });
        expect(get_events(events, reader_b) == std::vector{ event1 });

        events.update();

        should("after update, all events cleard") = [&] {
            expect(get_events(events, reader_missed) == std::vector<int>{});
        };

        // EventReader
        {

        }
    };
}