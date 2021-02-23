#pragma once

#ifndef NDEBUG
    #define IS_DEBUG
#endif

// DEBUG_ASSERT
#ifdef IS_DEBUG
    #include <cassert>
    #define DEBUG_ASSERT(x) assert(x) 
    #define DEBUG_ASSERT_EQ(x, y) assert(x == y)
    #define DEBUG_ASSERT_NEQ(x, y) assert(x != y)
#else
    #define DEBUG_ASSERT(x)
    #define DEBUG_ASSERT_EQ(x, y)
    #define DEBUG_ASSERT_NEQ(x, y)
#endif

// DEBUG_LOG
#ifdef IS_DEBUG
    #include <spdlog/spdlog.h>
    #define DEBUG_LOG(fmt, ...) spdlog::debug(fmt, __VA_ARGS__)
#else 
    #define DEBUG_LOG(...)
#endif