set(TEST_EXE tests_run)

set(TEST_SOURCES 
	"util-test/type_map-test.cpp" 
	"core-test/ecs-test/resource-test.cpp" 
	"core-test/ecs-test/system-test.cpp" 
	"core-test/ecs-test/scheduler-test.cpp" 
	"util-test/meta-test.cpp" 
	"util-test/rng-test.cpp" 
	"core-test/game-test/game-test.cpp" 
	"core-test/game-test/runner-test.cpp" 
	"util-test/common-test.cpp" 
	"core-test/assets-test/assets-test.cpp"
	"core-test/assets-test/handle-test.cpp"
	"util-test/sync-test/mutex-test.cpp"
	"util-test/sync-test/rwlock-test.cpp"
	"util-test/uuid-test.cpp"
	"util-test/memory-test/rc-test.cpp"
	"core-test/assets-test/asset_server-test.cpp"
	"core-test/assets-test/asset_io-test/asset_io_impl-test.cpp"
	"sdl-test/sdl_img_loader-test.cpp"
	"sdl-test/sdl_event-test.cpp"
	"core-test/game-test/events-test.cpp"
	"core-test/input-test/input-test.cpp"
	"core-test/input-test/mouse-test.cpp"
	)

include_directories(ut)

add_executable(${TEST_EXE} test_main.cpp ${TEST_SOURCES})

find_package(entt CONFIG REQUIRED)
target_link_libraries(${TEST_EXE} PRIVATE EnTT::EnTT)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)
target_link_libraries(${TEST_EXE} PRIVATE SDL2::SDL2)
target_link_libraries(${TEST_EXE} PRIVATE SDL2::SDL2_image)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${TEST_EXE} PRIVATE fmt::fmt)

# copy assets folder
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/assets")
file(COPY "${CMAKE_SOURCE_DIR}/tests/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")