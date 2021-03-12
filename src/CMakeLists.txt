set(EXECUTABLE_NAME CNGame)

add_executable(${EXECUTABLE_NAME} main.cpp)

# copy assets folder
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/assets")
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

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

# eigen3
find_package(eigen3 CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE Eigen3::Eigen)

# libsndfile
find_package(SndFile CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SndFile::sndfile)

# openal-soft
find_package(OpenAL CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE OpenAL::OpenAL)

# concurrentqueue
find_package(unofficial-concurrentqueue  CONFIG REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE unofficial::concurrentqueue::concurrentqueue)