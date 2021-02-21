# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)

project("Connor-Nathan-Game")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

set(EXECUTABLE_NAME CNGame)
add_executable(${EXECUTABLE_NAME} src/main.cpp)

# ------------
# dependencies

# fmt
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE fmt::fmt-header-only)

# spdlog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE spdlog::spdlog)

# entt
find_package(entt CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE EnTT::EnTT)

# sdl2
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL2::SDL2)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL2::SDL2main)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SDL2::SDL2_image)

# tl-<expected/optional>
find_package(tl-expected CONFIG REQUIRED)
find_package(tl-optional CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE tl::expected)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE tl::optional)
