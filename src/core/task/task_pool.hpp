#pragma once

#include <concepts>
#include <thread>
#include <util/common.hpp>

// TODO: Make this a resource
class TaskPool
{
public:
    template <std::invocable F>
    void execute(F&& f) const
    {
        auto t = std::thread(FWD(f));
        t.detach();
    }
};