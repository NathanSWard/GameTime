set(TEXT_EXE tests_run)
add_executable(${TEXT_EXE} test_main.cpp core-test/type_map-test.cpp)

find_package(entt CONFIG REQUIRED)
target_link_libraries(${TEXT_EXE} PRIVATE EnTT::EnTT)