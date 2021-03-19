#pragma once

#include <entt/entt.hpp>

template<typename Entity, typename Type>
struct entt::storage_traits<Entity, Type> {
    // prevents entt:: from using its internal signals.
    using storage_type = storage_adapter_mixin<basic_storage<Entity, Type>>;
};

using World = entt::registry;

using entity_t = entt::entity;

inline constexpr auto null_entity = entt::null;