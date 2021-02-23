set(TEXT_EXE tests_run)

set(TEST_SOURCES "util-test/type_map-test.cpp" "core-test/ecs-test/resource-test.cpp" "core-test/ecs-test/system-test.cpp" "util-test/meta-test.cpp" "util-test/rng-test.cpp")
add_executable(${TEXT_EXE} test_main.cpp ${TEST_SOURCES})

include_directories(ut)

find_package(entt CONFIG REQUIRED)
target_link_libraries(${TEXT_EXE} PRIVATE EnTT::EnTT)