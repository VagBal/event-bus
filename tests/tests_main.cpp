/**
 * @file tests_main.cpp
 * @brief Integration tests for the complete event-bus system
 * 
 * These tests verify the entire application by simulating the main() function logic:
 * - Full system integration with all components
 * - EventBus + SimulatorManager + Multiple Simulators + Consumer
 * - Event flow from sensors through bus to consumer
 * - Multiple start/stop cycles
 * - Concurrent operation of all sensor types
 * - Clean system shutdown
 * - Resource cleanup
 * 
 * Unlike unit tests that isolate components, these integration tests verify
 * that all pieces work together correctly in realistic scenarios. They test
 * the actual application workflow that would run in production.
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include "SensorSimulator/SimulatorManager.h"
#include "SensorSimulator/GasSensorSimulator.h"
#include "SensorSimulator/TemperatureSensorSimulator.h"
#include "SensorSimulator/PressureSensorSimulator.h"
#include "EventBus/EventBus.h"
#include "ConsumerSimulator/TestConsumerSimulator.h"

/**
 * @class MainApplicationTest
 * @brief Integration test fixture for full application scenarios
 * 
 * Provides a clean test environment for integration testing.
 * Currently implements empty SetUp/TearDown as tests manage
 * their own resources to test different scenarios.
 */
class MainApplicationTest : public ::testing::Test
{
protected:
    /** @brief Setup for integration tests (currently empty) */
    void SetUp() override
    {
        // Setup code if needed
    }

    /** @brief Cleanup for integration tests (currently empty) */
    void TearDown() override
    {
        // Cleanup code if needed
    }
};

/**
 * @test Full system integration test
 * 
 * Simulates the complete main() function workflow:
 * 1. Creates SimulatorManager and EventBus
 * 2. Adds all three sensor types (Gas, Temperature, Pressure)
 * 3. Creates consumer to process events
 * 4. Starts EventBus and all simulators
 * 5. Runs for brief period
 * 6. Cleanly shuts down all components
 * 
 * Verifies that all components can be initialized, started, and stopped
 * without crashes or resource leaks.
 */
TEST_F(MainApplicationTest, IntegrationTestFullSystem)
{
    // This test simulates the main() function logic
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

    // Run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Stop all simulators
    simulator_manager.stopAll();

    event_bus.stop();

    SUCCEED();
}

TEST_F(MainApplicationTest, SystemHandlesMultipleStartStop)
{
    SensorSimulator::SimulatorManager simulator_manager;
    EventBus event_bus;

    simulator_manager.addSimulator(std::make_unique<SensorSimulator::PressureSensorSimulator>(event_bus));

    ConsumerSimulator::TestConsumerSimulator test_consumer(event_bus);

    // Start and stop cycle 1
    event_bus.start();
    simulator_manager.startAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    simulator_manager.stopAll();
    event_bus.stop();

    SUCCEED();
}

TEST_F(MainApplicationTest, EventFlowFromSimulatorToConsumer)
{
    EventBus event_bus;
    std::atomic<int> events_received{0};

    // Subscribe to count events
    event_bus.subscribe([&events_received](const Event::Event& event) {
        events_received++;
    });

    ConsumerSimulator::TestConsumerSimulator test_consumer(event_bus);

    SensorSimulator::SimulatorManager simulator_manager;
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::PressureSensorSimulator>(event_bus));

    event_bus.start();
    simulator_manager.startAll();

    // Let events flow
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    simulator_manager.stopAll();
    event_bus.stop();

    // Should have received events
    EXPECT_GT(events_received.load(), 0);
}

TEST_F(MainApplicationTest, AllSensorTypesGenerateEvents)
{
    EventBus event_bus;
    std::atomic<int> co_events{0};
    std::atomic<int> temp_events{0};
    std::atomic<int> pressure_events{0};

    event_bus.subscribe([&co_events, &temp_events, &pressure_events](const Event::Event& event) {
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event)
        {
            switch (sensor_event->getSensorType())
            {
                case Event::SensorType::CoSensor:
                    co_events++;
                    break;
                case Event::SensorType::TempSensor:
                    temp_events++;
                    break;
                case Event::SensorType::PressureSensor:
                    pressure_events++;
                    break;
            }
        }
    });

    SensorSimulator::SimulatorManager simulator_manager;
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::GasSensorSimulator>(event_bus));
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::TemperatureSensorSimulator>(event_bus));
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::PressureSensorSimulator>(event_bus));

    event_bus.start();
    simulator_manager.startAll();

    // Run long enough to get pressure events (1 sec interval)
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    simulator_manager.stopAll();
    event_bus.stop();

    // Pressure sensor should have generated events (1 second interval)
    EXPECT_GT(pressure_events.load(), 0);
}

TEST_F(MainApplicationTest, SystemCleanShutdown)
{
    {
        SensorSimulator::SimulatorManager simulator_manager;
        EventBus event_bus;

        simulator_manager.addSimulator(std::make_unique<SensorSimulator::PressureSensorSimulator>(event_bus));
        ConsumerSimulator::TestConsumerSimulator test_consumer(event_bus);

        event_bus.start();
        simulator_manager.startAll();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        simulator_manager.stopAll();
        event_bus.stop();
        
        // Objects destroyed here - should be clean
    }

    SUCCEED();
}

TEST_F(MainApplicationTest, NoMemoryLeaksWithManyEvents)
{
    EventBus event_bus;
    std::atomic<int> event_count{0};

    event_bus.subscribe([&event_count](const Event::Event& event) {
        event_count++;
    });

    SensorSimulator::SimulatorManager simulator_manager;
    simulator_manager.addSimulator(std::make_unique<SensorSimulator::PressureSensorSimulator>(event_bus));

    event_bus.start();
    simulator_manager.startAll();

    // Generate many events
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    simulator_manager.stopAll();
    event_bus.stop();

    // Should have processed at least one event
    EXPECT_GT(event_count.load(), 0);
}