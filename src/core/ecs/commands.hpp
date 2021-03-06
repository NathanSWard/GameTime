#pragma once

#include "util/common.hpp"
#include "resource.hpp"
#include "world.hpp"

template <typename T>
concept Bundle = requires(T && bundle, World & world)
{
    { FWD(bundle).build(world) };
};

// TODO:
//     Lazy commands??
//     vector<unique_ptr<Command>>
class Commands {
    Resources* m_resources;
    World* m_world;
    entity_t m_curr_entity = null_entity;

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

    auto spawn(entity_t* const e = nullptr) -> Commands&
    {
        auto const entity = m_world->create();
        m_curr_entity = entity;
        if (e) {
            *e = entity;
        }
        return *this;
    }

    template <Bundle B>
    auto spawn(B&& bundle) -> Commands&
    {
        FWD(bundle).build(*m_world);
        return *this;
    }

    auto set_current_entity(entity_t const e) -> Commands&
    {
        m_curr_entity = e;
        return *this;
    }

    auto current_entity() const noexcept -> entity_t
    {
        return m_curr_entity;
    }

    auto despawn(entity_t const e) -> Commands&
    {
        m_world->destroy(e);
        return *this;
    }

    auto clear_entities() -> Commands&
    {
        m_world->clear();
        return *this;
    }

    template <typename T, typename... Args>
    auto add_component(Args&&... args) -> Commands&
    {
        DEBUG_ASSERT(m_curr_entity != null_entity, "`Commands` does not contain a vlail entity. Consider using `spawn()` to set one.");
        m_world->emplace<T>(m_curr_entity, FWD(args)...);
        return *this;
    }

    template <typename... Cs>
    auto remove_components() -> Commands&
    {
        m_world->remove<Cs...>(m_curr_entity);
        return *this;
    }

    template <typename R, typename... Args>
    auto try_add_resource(Args&&... args) -> Resource<R>
    {
        return m_resources->try_add_resource<R>(FWD(args)...);
    }

    template <typename R, typename... Args>
    auto set_resource(Args&&... args) -> Resource<R>
    {
        return m_resources->set_resource<R>(FWD(args)...);
    }

    template <typename R>
    auto remove_resource() -> std::unique_ptr<R>
    {
        return m_resources->remove_resource<R>();
    }
};