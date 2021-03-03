#pragma once

#include <debug/debug.hpp>
#include <core/ecs/resource.hpp>
#include <util/common.hpp>
#include <util/ranges/chain.hpp>
#include <ranges>
#include <vector>

template <typename T>
struct EventId 
{
    std::size_t id = 0;
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

namespace {

    enum class State
    {
        A, B,
    };

} // namespace

template <typename T>
class Events
{
    std::vector<Event<T>> m_a_events;
    std::vector<Event<T>> m_b_events;
    std::size_t m_a_start_event_count = 0;
    std::size_t m_b_start_event_count = 0;
    std::size_t m_event_count = 0;
    State m_state = State::A;

    template <typename F>
    auto internal_event_reader(std::size_t& last_event_count, F&& f) const noexcept
    {
        std::size_t const a_index = last_event_count > m_a_start_event_count ? 
            last_event_count - m_a_start_event_count : 0;
        std::size_t const b_index = last_event_count > m_b_start_event_count ?
            last_event_count - m_b_start_event_count : 0;

        last_event_count = m_event_count;

        auto a_view = m_a_events | std::views::drop(a_index) | std::views::transform(f);
        auto b_view = m_b_events | std::views::drop(b_index) | std::views::transform(f);

        switch (m_state) {
            case State::A:
                return chain(b_view, a_view);
            default: // State::B
                return chain(a_view, b_view);
        }
    }

    template <typename>
    friend class ManualEventReader;
    template <typename>
    friend class EventReader;

public:
    template <typename... Args>
    void send(Args&&... args)
    {
        auto const id = EventId<T>{ .id = m_event_count };

        switch (m_state) {
            case State::A: 
                m_a_events.emplace_back(id, FWD(args)...);
                break;
            case State::B: 
                m_b_events.emplace_back(id, FWD(args)...);
                break;
            default: // unreachable
                break;
        }

        ++m_event_count;
    }

    // includes all events already in the event buffers.
    constexpr auto get_reader() const noexcept -> ManualEventReader<T>;

    // ignores all events already in the event buffer.
    constexpr auto get_reader_current() const noexcept -> ManualEventReader<T>;

    void update() 
    { 
        switch (m_state) {
            case State::A: {
                std::vector<Event<T>>().swap(m_b_events);
                m_state = State::B;
                m_b_start_event_count = m_event_count;
            }
                break;
            case State::B: {
                std::vector<Event<T>>().swap(m_a_events);
                m_state = State::A;
                m_a_start_event_count = m_event_count;
            }
                break;
            default: // unreachable
                break;
        }
    }

    void clear() 
    { 
        m_a_events.clear();
        m_b_events.clear();
    }
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
        return events.internal_event_reader(m_last_event_count, map_instance_event_with_id<T>);
    }

    auto iter(Events<T> const& events) noexcept
    {
        return events.internal_event_reader(m_last_event_count, map_instance_event<T>);
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
        return m_events->internal_event_reader(m_last_event_count->count, map_instance_event_with_id<T>);
    }

    auto iter() noexcept
    {
        return m_events->internal_event_reader(m_last_event_count->count, map_instance_event<T>);
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
    return ManualEventReader<T>{ m_event_count };
}