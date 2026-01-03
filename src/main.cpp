#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

#include "SensorSimulator/SimulatorManager.h"
#include "SensorSimulator/GasSensorSimulator.h"
#include "SensorSimulator/TemperatureSensorSimulator.h"
#include "SensorSimulator/PressureSensorSimulator.h"
#include "EventBus/EventBus.h"
#include "ConsumerSimulator/TestConsumerSimulator.h"

static volatile std::sig_atomic_t g_stop_requested = 0;

static void onSignal(int)
{
    g_stop_requested = 1; // Ctrl+C sets this
}

int main(int argc, const char** argv)
{
    std::signal(SIGINT, onSignal);  // Ctrl+C
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

    const auto start = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(60);

    while (!g_stop_requested &&
           (std::chrono::steady_clock::now() - start) < timeout)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\r\nStopping simulations..." << "\n";

    // Stop all simulators
    simulator_manager.stopAll();

    event_bus.stop();

    return 0;
}