include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unittests
    tests_main.cpp
)

target_include_directories(unittests PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(unittests
    gtest_main
	pthread
)

include(GoogleTest)
gtest_discover_tests(unittests)

# Example test, replace with your test files
# add_executable(kv-db_test test_main.cpp)
# target_link_libraries(kv-db_test gtest_main)
# add_test(NAME kv-db_test COMMAND kv-db_test)

# Automatically discover tests if using gtest_discover_tests
# gtest_discover_tests(kv-db_test)
