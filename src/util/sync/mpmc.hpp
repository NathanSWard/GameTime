#pragma once

#include <memory>
#include <tbb/concurrent_queue.h>
#include <tl/optional.hpp>
#include <util/common.hpp>

template <typename T>
requires (std::is_default_constructible_v<T>)
class Receiver;

template <typename T>
class Sender
{
    std::shared_ptr<tbb::concurrent_queue<T>> m_inner;

    Sender(std::shared_ptr<tbb::concurrent_queue<T>> ptr) noexcept
        : m_inner(MOV(ptr))
    {}

    template <typename U>
    friend auto mpmc_channel() -> std::tuple<Sender<U>, Receiver<U>>;

public:
    Sender(Sender&&) noexcept = default;
    Sender& operator=(Sender&&) noexcept = default;
    Sender(Sender const&) = default;
    Sender& operator=(Sender const&) noexcept = default;

    template <typename... Args>
    void send(Args&&... args)
    {
        m_inner->emplace(FWD(args)...);
    }
};

template <typename T>
requires (std::is_default_constructible_v<T>)
class Receiver
{
    std::shared_ptr<tbb::concurrent_queue<T>> m_inner;

    Receiver(std::shared_ptr<tbb::concurrent_queue<T>> ptr) noexcept
        : m_inner(MOV(ptr))
    {}

    template <typename U>
    friend auto mpmc_channel()->std::tuple<Sender<U>, Receiver<U>>;

public:
    Receiver(Receiver&&) noexcept = default;
    Receiver& operator=(Receiver&&) noexcept = default;

    [[nodiscard]] auto recv() -> tl::optional<T>
    {
        T t{};
        if (m_inner->try_pop(t)) {
            return tl::make_optional<T>(MOV(t));
        }
        return {};
    }
};

template <typename T>
[[nodiscard]] auto mpmc_channel() -> std::tuple<Sender<T>, Receiver<T>>
{
    auto queue = std::make_shared<tbb::concurrent_queue<T>>();
    auto sender = Sender(queue);
    return std::tuple<Sender<T>, Receiver<T>>{ MOV(sender), Receiver<T>(MOV(queue)) };
}