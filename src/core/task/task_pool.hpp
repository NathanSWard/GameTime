#pragma once

#include <concepts>
#include <future>
#include <util/common.hpp>

class TaskPool
{
public:
    template <std::invocable F>
    [[nodiscard]] auto execute(F&& f) const
    {
        return std::async(std::launch::async, FWD(f));
    }
};