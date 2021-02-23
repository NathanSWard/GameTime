#pragma once

#ifndef NDEBUG
    #define IS_DEBUG
#endif

// DEBUG_ASSERT
#if IS_DEBUG
    #include <cassert>
    #define DEBUG_ASSERT(x) assert(x) 
#else
    #define DEBUG_ASSERT(x)
#endif

// DEBUG_LOG
#if IS_DEBUG
    #include <spdlog/spdlog.h>
    #define DEBUG_LOG(fmt, ...) spdlog::debug(fmt, __VA_ARGS__)
#else 
    #define DEBUG_LOG(...)
#endif