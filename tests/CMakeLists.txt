find_package(Catch2 REQUIRED)
add_executable(tests WIN32 test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2)
target_include_directories(tests
    PUBLIC
        $<INSTALL_INTERFACE:src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)
