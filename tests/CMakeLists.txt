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
    tests_randomNumberGenerator.cpp
    tests_sensorEvent.cpp
    tests_eventBus.cpp
    tests_testConsumerSimulator.cpp
    tests_simulatorManager.cpp
    tests_genericSimulator.cpp
    ${CMAKE_SOURCE_DIR}/src/Event/SensorEvent.cpp
    ${CMAKE_SOURCE_DIR}/src/EventBus/EventBus.cpp
    ${CMAKE_SOURCE_DIR}/src/SensorSimulator/SimulatorManager.cpp
    ${CMAKE_SOURCE_DIR}/src/ConsumerSimulator/TestConsumerSimulator.cpp
)

target_include_directories(unittests PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(unittests
    gtest_main
    gmock
	pthread
)

# Enable coverage flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(unittests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(unittests PRIVATE --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(unittests)