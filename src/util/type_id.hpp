#pragma once

using typeid_t = void(*)();

template <typename T>
typeid_t type_id() noexcept
{
    return typeid_t(type_id<T>);
}