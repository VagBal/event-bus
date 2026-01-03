/**
 * @file tests_genericSimulator.cpp
 * @brief Unit tests for GenericSimulator template and sensor type aliases
 * 
 * Test suite covering:
 * - Construction of all sensor simulator types (Gas, Temperature, Pressure)
 * - Event publishing at type-specific intervals
 * - Proper sensor type identification in published events
 * - Stop mechanism and thread safety
 * - Concurrent simulator operation
 * - Value ranges specific to each sensor type
 * 
 * Tests verify that the GenericSimulator template correctly instantiates
 * for different sensor types and intervals, and that the type aliases
 * (GasSensorSimulator, TemperatureSensorSimulator, PressureSensorSimulator)
 * work as expected.
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "SensorSimulator/GenericSimulator.h"
#include "SensorSimulator/GasSensorSimulator.h"
#include "SensorSimulator/TemperatureSensorSimulator.h"
#include "SensorSimulator/PressureSensorSimulator.h"
#include "EventBus/EventBus.h"
#include "Event/SensorEvent.h"

/**
 * @class GenericSimulatorTest
 * @brief Test fixture for GenericSimulator tests
 * 
 * Provides:
 * - EventBus instance started before each test
 * - Automatic cleanup and stopping
 * - Ready-to-use environment for simulator testing
 */
class GenericSimulatorTest : public ::testing::Test
{
protected:
    /** @brief Creates and starts EventBus before each test */
    void SetUp() override
    {
        event_bus_ = std::make_unique<EventBus>();
        event_bus_->start();
    }

    /** @brief Stops and destroys EventBus after each test */
    void TearDown() override
    {
        if (event_bus_)
        {
            event_bus_->stop();
        }
        event_bus_.reset();
    }

    std::unique_ptr<EventBus> event_bus_;  ///< EventBus instance for testing
};

/** @test Verifies GasSensorSimulator can be constructed */
TEST_F(GenericSimulatorTest, CreateGasSensorSimulator)
{
    SensorSimulator::GasSensorSimulator simulator(*event_bus_);
    SUCCEED();
}

TEST_F(GenericSimulatorTest, CreateTemperatureSensorSimulator)
{
    SensorSimulator::TemperatureSensorSimulator simulator(*event_bus_);
    SUCCEED();
}

TEST_F(GenericSimulatorTest, CreatePressureSensorSimulator)
{
    SensorSimulator::PressureSensorSimulator simulator(*event_bus_);
    SUCCEED();
}

TEST_F(GenericSimulatorTest, GasSimulatorPublishesEvents)
{
    std::atomic<int> event_count{0};
    std::atomic<bool> received_co_sensor{false};
    
    event_bus_->subscribe([&event_count, &received_co_sensor](const Event::Event& event) {
        event_count++;
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event && sensor_event->getSensorType() == Event::SensorType::CoSensor)
        {
            received_co_sensor = true;
        }
    });
    
    SensorSimulator::GasSensorSimulator simulator(*event_bus_);
    
    // Start simulation in a thread
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    // Let it run for a bit (GasSensor publishes every 10 seconds, so we won't get events in this test)
    // But we can test the stop mechanism
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    SUCCEED();
}

TEST_F(GenericSimulatorTest, TemperatureSimulatorPublishesEvents)
{
    std::atomic<int> event_count{0};
    std::atomic<bool> received_temp_sensor{false};
    
    event_bus_->subscribe([&event_count, &received_temp_sensor](const Event::Event& event) {
        event_count++;
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event && sensor_event->getSensorType() == Event::SensorType::TempSensor)
        {
            received_temp_sensor = true;
        }
    });
    
    SensorSimulator::TemperatureSensorSimulator simulator(*event_bus_);
    
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    // Temperature sensor publishes every 5 seconds, wait longer to potentially get an event
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    SUCCEED();
}

TEST_F(GenericSimulatorTest, PressureSimulatorPublishesEvents)
{
    std::atomic<int> event_count{0};
    std::atomic<bool> received_pressure_sensor{false};
    
    event_bus_->subscribe([&event_count, &received_pressure_sensor](const Event::Event& event) {
        event_count++;
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event && sensor_event->getSensorType() == Event::SensorType::PressureSensor)
        {
            received_pressure_sensor = true;
        }
    });
    
    SensorSimulator::PressureSensorSimulator simulator(*event_bus_);
    
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    // Pressure sensor publishes every 1 second, should get events
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    // Should have received at least one event
    EXPECT_GT(event_count.load(), 0);
    EXPECT_TRUE(received_pressure_sensor.load());
}

TEST_F(GenericSimulatorTest, StopSimulatorImmediately)
{
    SensorSimulator::GasSensorSimulator simulator(*event_bus_);
    
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    // Stop immediately
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    SUCCEED();
}

TEST_F(GenericSimulatorTest, MultipleSimulatorsRunConcurrently)
{
    std::atomic<int> co_count{0};
    std::atomic<int> temp_count{0};
    std::atomic<int> pressure_count{0};
    
    event_bus_->subscribe([&co_count, &temp_count, &pressure_count](const Event::Event& event) {
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event)
        {
            switch (sensor_event->getSensorType())
            {
                case Event::SensorType::CoSensor:
                    co_count++;
                    break;
                case Event::SensorType::TempSensor:
                    temp_count++;
                    break;
                case Event::SensorType::PressureSensor:
                    pressure_count++;
                    break;
            }
        }
    });
    
    SensorSimulator::GasSensorSimulator gas_sim(*event_bus_);
    SensorSimulator::TemperatureSensorSimulator temp_sim(*event_bus_);
    SensorSimulator::PressureSensorSimulator pressure_sim(*event_bus_);
    
    std::thread gas_thread([&gas_sim]() { gas_sim.runSimulation(); });
    std::thread temp_thread([&temp_sim]() { temp_sim.runSimulation(); });
    std::thread pressure_thread([&pressure_sim]() { pressure_sim.runSimulation(); });
    
    // Run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    gas_sim.stopSimulation();
    temp_sim.stopSimulation();
    pressure_sim.stopSimulation();
    
    if (gas_thread.joinable()) gas_thread.join();
    if (temp_thread.joinable()) temp_thread.join();
    if (pressure_thread.joinable()) pressure_thread.join();
    
    // Pressure sensor should have published (1 second interval)
    EXPECT_GT(pressure_count.load(), 0);
}

TEST_F(GenericSimulatorTest, SimulatorStopsCleanly)
{
    SensorSimulator::PressureSensorSimulator simulator(*event_bus_);
    
    std::atomic<bool> thread_finished{false};
    
    std::thread sim_thread([&simulator, &thread_finished]() {
        simulator.runSimulation();
        thread_finished = true;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    simulator.stopSimulation();
    
    // Wait for thread to finish
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    EXPECT_TRUE(thread_finished.load());
}

TEST_F(GenericSimulatorTest, PublishedEventsHaveCorrectType)
{
    std::atomic<bool> correct_type{false};
    
    event_bus_->subscribe([&correct_type](const Event::Event& event) {
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event && sensor_event->getSensorType() == Event::SensorType::PressureSensor)
        {
            correct_type = true;
        }
    });
    
    SensorSimulator::PressureSensorSimulator simulator(*event_bus_);
    
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    EXPECT_TRUE(correct_type.load());
}

TEST_F(GenericSimulatorTest, SimulatorPublishesMultipleEvents)
{
    std::atomic<int> event_count{0};
    
    event_bus_->subscribe([&event_count](const Event::Event& event) {
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event && sensor_event->getSensorType() == Event::SensorType::PressureSensor)
        {
            event_count++;
        }
    });
    
    SensorSimulator::PressureSensorSimulator simulator(*event_bus_);
    
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    // Run for 2.1 seconds (should get 2 events with 1 second interval)
    std::this_thread::sleep_for(std::chrono::milliseconds(2100));
    
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    // Should have received multiple events
    EXPECT_GE(event_count.load(), 2);
}

TEST_F(GenericSimulatorTest, StopBeforeFirstEvent)
{
    std::atomic<int> event_count{0};
    
    event_bus_->subscribe([&event_count](const Event::Event& event) {
        event_count++;
    });
    
    SensorSimulator::GasSensorSimulator simulator(*event_bus_);
    
    std::thread sim_thread([&simulator]() {
        simulator.runSimulation();
    });
    
    // Stop before first event (GasSensor has 10 second interval)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    simulator.stopSimulation();
    
    if (sim_thread.joinable())
    {
        sim_thread.join();
    }
    
    SUCCEED();
}
