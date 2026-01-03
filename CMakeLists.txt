cmake_minimum_required(VERSION 3.14)
project(event-bus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(event-bus 
    src/main.cpp
    src/Event/SensorEvent.cpp
    src/EventBus/EventBus.cpp
    src/SensorSimulator/TemperatureSensorSimulator.cpp
    src/SensorSimulator/PressureSensorSimulator.cpp
    src/SensorSimulator/GasSensorSimulator.cpp
    src/SensorSimulator/SimulatorManager.cpp
    src/ConsumerSimulator/TestConsumerSimulator.cpp
)

# Enable testing
enable_testing()
add_subdirectory(tests)
