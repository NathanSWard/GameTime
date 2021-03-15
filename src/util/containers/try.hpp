#pragma once

template <typename T>
struct try_ops;

#define TRY(VAR_NAME, VAL_TO_TRY) \
    static_assert(std::is_rvalue_reference_v<decltype(VAL_TO_TRY)>, \
        "TRY(x, y) macro requires `y` to be an rvalue reference. Consider using `std::move`"); \
    if (try_ops<std::remove_cvref_t<decltype(VAL_TO_TRY)>>::should_early_return(VAL_TO_TRY)) { \
        return try_ops<std::remove_cvref_t<decltype(VAL_TO_TRY)>>::early_return(MOV(VAL_TO_TRY)); \
    } \
    auto VAR_NAME = try_ops<std::remove_cvref_t<decltype(VAL_TO_TRY)>>::extract_value(MOV(VAL_TO_TRY));
