#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace container_detail {
    template <typename T>
    using default_hash_t = decltype(std::declval<absl::flat_hash_set<T>>().hash_function());

    template <typename T>
    using default_eq_t = decltype(std::declval<absl::flat_hash_set<T>>().key_eq());

    template <typename T>
    using default_alloc_t = std::allocator<T>;
}

template <
        typename Key,
        typename Value,
        typename Hash = container_detail::default_hash_t<Key>,
        typename Eq = container_detail::default_eq_t<Key>,
        typename Alloc = container_detail::default_alloc_t<std::pair<Key const, Value>>>
using HashMap = absl::flat_hash_map<Key, Value, Hash, Eq, Alloc>;

template <
        typename T,
        typename Hash = container_detail::default_hash_t<T>,
        typename Eq = container_detail::default_eq_t<T>,
        typename Alloc = container_detail::default_alloc_t<T>>
using HashSet = absl::flat_hash_set<T, Hash, Eq, Alloc>;

