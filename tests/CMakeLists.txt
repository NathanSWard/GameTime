set(TEXT_EXE tests_run)

set(TEST_SOURCES 
	"util-test/type_map-test.cpp" 
	"core-test/ecs-test/resource-test.cpp" 
	"core-test/ecs-test/system-test.cpp" 
	"util-test/meta-test.cpp" 
	"util-test/rng-test.cpp" 
	"core-test/game-test.cpp" 
	"util-test/common-test.cpp" 
	"core-test/assets-test/assets-test.cpp"
	"core-test/assets-test/handle-test.cpp"
	"util-test/sync-test/mutex-test.cpp"
	"util-test/sync-test/rwlock-test.cpp"
	"util-test/uuid-test.cpp"
	"util-test/memory-test/rc-test.cpp"
	)

include_directories(ut)

add_executable(${TEXT_EXE} test_main.cpp ${TEST_SOURCES})

find_package(entt CONFIG REQUIRED)
target_link_libraries(${TEXT_EXE} PRIVATE EnTT::EnTT)