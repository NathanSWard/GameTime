#pragma once

#include "resource.hpp"
#include "system.hpp"
#include "world.hpp"

#include <debug/debug.hpp>
#include <util/rng.hpp>

#include <unordered_map>
#include <vector>

// TODO: Make the scheduler more involved
//       Allow for systems to run before others.
class Scheduler
{
public:
    using system_id_t = std::size_t;

private:
    std::unordered_map<system_id_t, System> m_systems;

public:
    template <typename F>
    auto add_system(F&& func) -> system_id_t
    {
        auto system = System::create(std::forward<F>(func));
        
        auto const id = [this] {
            for (;;) {
                auto const id = util::uniform_rand<system_id_t>();
                if (!m_systems.contains(id)) {
                    return id;
                }
            }
        }();

        auto const [iter, inserted] = m_systems.try_emplace(id, system);
        DEBUG_ASSERT(inserted);
        return id;
    }

    auto remove_system(system_id_t const id) -> bool
    {
        if (auto const iter = m_systems.find(id); iter != m_systems.end()) {
            m_systems.erase(iter);
            return true;
        }
        return false;
    }

    void run_systems(Resources& resources, World& world)
    {
        for (auto& [key, val] : m_systems) {
            val.run(resources, world);
        }
    }
};