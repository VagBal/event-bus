#include <iostream>
#include <future>


#include "SensorSimulator/SimulatorManager.h"
#include "SensorSimulator/GasSensorSimulator.h"
#include "SensorSimulator/TemperatureSensorSimulator.h"
#include "SensorSimulator/PressureSensorSimulator.h"
#include "EventBus/EventBus.h"
#include "ConsumerSimulator/TestConsumerSimulator.h"

int main(int argc, const char** argv)
{
    SensorSimulator::SimulatorManager simulator_manager;
    EventBus event_bus;

    // Add simulators to the manager
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::GasSensorSimulator>(event_bus));
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::TemperatureSensorSimulator>(event_bus));
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::PressureSensorSimulator>(event_bus));

    ConsumerSimulator::TestConsumerSimulator test_consumer(event_bus);

    event_bus.start();

    // Start all simulators
    simulator_manager.startAll();

    // Wait for termination signal (e.g., CTRL+C)
    auto input_feature = std::async(std::launch::async, []() {
        std::cin.get();
    });

    if (input_feature.wait_for(std::chrono::seconds(60)) == std::future_status::timeout) {
        std::cout << "\r\nTimeout reached, stopping simulations..." << "\n";
    } else {
        std::cout << "\r\nUser requested stop, stopping simulations..." << "\n";
    }

    // Stop all simulators
    simulator_manager.stopAll();

    event_bus.stop();

    return 0;
}