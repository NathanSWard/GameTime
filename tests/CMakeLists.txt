set(TEXT_EXE tests_run)

set(TEST_SOURCES "core-test/type_map-test.cpp" "core-test/resource-test.cpp" "core-test/system-test.cpp" "core-test/meta-test.cpp" "util-test/rng-test.cpp")
add_executable(${TEXT_EXE} test_main.cpp ${TEST_SOURCES})

find_package(entt CONFIG REQUIRED)
target_link_libraries(${TEXT_EXE} PRIVATE EnTT::EnTT)