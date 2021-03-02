#pragma once

#include <debug/debug.hpp>
#include <core/ecs/resource.hpp>
#include <util/common.hpp>
#include <ranges>
#include <vector>

template <typename T>
struct EventId 
{
    std::uint64_t id = 0;
};

template <typename T>
struct Event 
{
    template <typename... Args>
    constexpr Event(EventId<T> const id, Args&&... args)
        : event_id(id)
        , value(FWD(args)...)
    {}

    constexpr Event(Event&&) noexcept = default;
    constexpr Event& operator=(Event&&) noexcept = default;

    EventId<T> event_id;
    T value;
};

template <typename T>
class ManualEventReader;

namespace {
    template <typename T>
    constexpr auto map_instance_event_with_id(Event<T> const& event) noexcept -> std::tuple<EventId<T>, T const&>
    {
        return std::make_tuple(EventId<T> {event.event_id}, std::cref(event.value));
    }

    template <typename T>
    constexpr auto map_instance_event(Event<T> const& event) noexcept -> T const&
    {
        return event.value;
    }
}

// TODO: possible double buffering
template <typename T>
class Events
{
    std::vector<Event<T>> m_events;

    auto internal_event_reader(std::size_t& last_event_count) const noexcept
    {
        auto view = m_events | std::views::drop(last_event_count) | std::views::transform(map_instance_event_with_id<T>);
        last_event_count = m_events.size();
        return view;
    }

    template <typename>
    friend class ManualEventReader;
    template <typename>
    friend class EventReader;

public:

    template <typename... Args>
    void send(Args&&... args)
    {
        auto const id = EventId<T>{static_cast<std::uint64_t>(m_events.size())};
        m_events.emplace_back(id, FWD(args)...);
    }

    // includes all events already in the event buffers.
    constexpr auto get_reader() const noexcept -> ManualEventReader<T>;

    // ignores all events already in the event buffer.
    constexpr auto get_reader_current() const noexcept -> ManualEventReader<T>;

    void update() { m_events.clear(); }

    void clear() { m_events.clear(); }
};

template <typename T>
void events_update_system(Resource<Events<T>> events)
{
    events->update();
}

template <typename T>
class ManualEventReader
{
    std::size_t m_last_event_count = 0;

public:
    constexpr ManualEventReader(std::size_t const last_event_count = 0) noexcept
        : m_last_event_count(last_event_count)
    {}

    constexpr ManualEventReader(ManualEventReader&&) noexcept = default;
    constexpr ManualEventReader(ManualEventReader const&) noexcept = default;
    constexpr ManualEventReader& operator=(ManualEventReader&&) noexcept = default;
    constexpr ManualEventReader& operator=(ManualEventReader const&) noexcept = default;

    auto iter_with_id(Events<T> const& events) noexcept
    {
        return events.internal_event_reader(m_last_event_count);
    }

    auto iter(Events<T> const& events) noexcept
    {
        return iter_with_id(events) | std::views::transform([](auto const& e) -> T const& { return std::get<1>(e); });
    }

};

template <typename T>
class EventReader
{
public:
    struct EventCount { std::size_t count = 0; };

private:
    Local<EventCount> m_last_event_count;
    Resource<Events<T> const> m_events;

public:
    constexpr EventReader(Local<EventCount> const& last_event_count, Resource<Events<T> const> events) noexcept
        : m_last_event_count(last_event_count)
        , m_events(MOV(events))
    {}

    constexpr EventReader(EventReader&&) noexcept = default;
    constexpr EventReader& operator=(EventReader&&) noexcept = default;

    EventReader(EventReader const&) = delete;
    EventReader& operator=(EventReader const&) = delete;

    auto iter_with_id() noexcept
    {
        return m_events->internal_event_reader(m_last_event_count->count);
    }

    auto iter() noexcept
    {
        return iter_with_id() | std::views::transform([](auto const& e) -> T const& { return std::get<1>(e); });
    }
};

template <typename T>
constexpr auto Events<T>::get_reader() const noexcept -> ManualEventReader<T>
{
    return ManualEventReader<T>{0};
}

template <typename T>
constexpr auto Events<T>::get_reader_current() const noexcept -> ManualEventReader<T>
{
    return ManualEventReader<T>{m_events.size()};
}