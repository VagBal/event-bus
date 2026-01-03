cmake_minimum_required(VERSION 3.14)
project(event-bus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_GPROF "Enable gprof profiling (-pg) for the event-bus binary" OFF)

include_directories(include)

add_executable(event-bus 
    src/main.cpp
    src/Event/SensorEvent.cpp
    src/EventBus/EventBus.cpp
    src/SensorSimulator/SimulatorManager.cpp
    src/ConsumerSimulator/TestConsumerSimulator.cpp
)

if(ENABLE_GPROF)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(event-bus PRIVATE -pg)
        target_link_options(event-bus PRIVATE -pg)
    else()
        message(WARNING "ENABLE_GPROF is ON, but compiler '${CMAKE_CXX_COMPILER_ID}' may not support -pg")
    endif()
endif()

# Enable testing
enable_testing()
add_subdirectory(tests)
