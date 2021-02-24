#pragma once

#include "util/common.hpp"
#include "resource.hpp"
#include "world.hpp"

// TODO:
//     Lazy commands??
//     vector<unique_ptr<Command>>
class Commands {
    Resources* m_resources;
    World* m_world;

public:

    constexpr Commands(Resources& resources, World& world) noexcept
        : m_resources(&resources)
        , m_world(&world)
    {
    }

    constexpr Commands(Commands&&) noexcept = default;
    constexpr Commands(Commands const&) noexcept = default;
    constexpr Commands& operator=(Commands&&) noexcept = default;
    constexpr Commands& operator=(Commands const&) noexcept = default;

    [[nodiscard]] auto spawn() -> entity_t
    {
        return m_world->create();
    }

    void despawn(entity_t const e)
    {
        m_world->destroy(e);
    }

    void clear_entities()
    {
        m_world->clear();
    }

    template <typename T, typename... Args>
    auto add_component(entity_t const e, Args&&... args) -> decltype(auto)
    {
        return m_world->emplace<T>(e, FWD(args)...);
    }

    template <typename... Cs>
    void remove_components(entity_t const e)
    {
        m_world->remove<Cs...>(e);
    }

    template <typename R, typename... Args>
    auto add_resource(Args&&... args) -> Resource<R>
    {
        return m_resources->add_resource<R>(FWD(args)...);
    }

    template <typename R>
    auto remove_resource() -> std::unique_ptr<R>
    {
        return m_resources->remove_resource<R>();
    }
};