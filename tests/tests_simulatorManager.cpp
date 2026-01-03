/**
 * @file tests_simulatorManager.cpp
 * @brief Unit tests for the SimulatorManager class
 * 
 * Comprehensive test suite covering:
 * - Adding simulators to the manager
 * - Starting all simulators concurrently
 * - Stopping all simulators and joining threads
 * - State management (cannot add while running)
 * - Concurrent simulator execution
 * - Multiple start/stop cycles
 * - Destructor cleanup
 * 
 * Uses Google Mock to create MockSensorSimulator for controlled testing
 * without actual sensor simulation delays. Mock expectations verify that
 * the manager correctly calls runSimulation() and stopSimulation() on
 * managed simulators.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include "SensorSimulator/SimulatorManager.h"
#include "SensorSimulator/ISensorSimulator.h"
#include "SensorSimulator/GasSensorSimulator.h"
#include "SensorSimulator/TemperatureSensorSimulator.h"
#include "SensorSimulator/PressureSensorSimulator.h"
#include "EventBus/EventBus.h"

/**
 * @class MockSensorSimulator
 * @brief Mock implementation of ISensorSimulator for testing
 * 
 * Provides controlled simulation behavior:
 * - runSimulation() blocks until stopSimulation() is called
 * - Allows testing of manager's thread coordination
 * - No actual sensor event generation or delays
 */
class MockSensorSimulator : public SensorSimulator::ISensorSimulator
{
public:
    MOCK_METHOD(void, runSimulation, (), (override));
    MOCK_METHOD(void, stopSimulation, (), (override));
    
    /**
     * @brief Mock implementation that blocks until stop is requested
     * Use with EXPECT_CALL to provide actual behavior during tests
     */
    void runSimulationImpl()
    {
        while (!should_stop_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    /**
     * @brief Mock implementation that signals runSimulation to exit
     * Use with EXPECT_CALL to provide actual behavior during tests
     */
    void stopSimulationImpl()
    {
        should_stop_ = true;
    }
    
private:
    std::atomic<bool> should_stop_{false};  ///< Flag to control simulation loop
};

/**
 * @class SimulatorManagerTest
 * @brief Test fixture for SimulatorManager tests
 * 
 * Provides:
 * - Fresh SimulatorManager instance for each test
 * - Automatic cleanup (stops all simulators)
 * - Consistent test environment
 */
class SimulatorManagerTest : public ::testing::Test
{
protected:
    /** @brief Creates a fresh SimulatorManager before each test */
    void SetUp() override
    {
        manager_ = std::make_unique<SensorSimulator::SimulatorManager>();
    }

    /** @brief Stops and destroys manager after each test */
    void TearDown() override
    {
        if (manager_)
        {
            manager_->stopAll();
        }
        manager_.reset();
    }

    std::unique_ptr<SensorSimulator::SimulatorManager> manager_;  ///< Manager instance under test
};

/** @test Verifies basic construction and destruction without crashes */
TEST_F(SimulatorManagerTest, ConstructAndDestruct)
{
    SensorSimulator::SimulatorManager manager;
    // Should construct and destruct without issues
    SUCCEED();
}

TEST_F(SimulatorManagerTest, AddSimulator)
{
    auto simulator = std::make_unique<MockSensorSimulator>();
    manager_->addSimulator(std::move(simulator));
    
    SUCCEED();
}

TEST_F(SimulatorManagerTest, AddMultipleSimulators)
{
    for (int i = 0; i < 3; ++i)
    {
        auto simulator = std::make_unique<MockSensorSimulator>();
        manager_->addSimulator(std::move(simulator));
    }
    
    SUCCEED();
}

TEST_F(SimulatorManagerTest, StartAllSimulators)
{
    testing::internal::CaptureStdout();
    
    auto simulator1 = std::make_unique<MockSensorSimulator>();
    auto simulator2 = std::make_unique<MockSensorSimulator>();
    
    auto* sim1_ptr = simulator1.get();
    auto* sim2_ptr = simulator2.get();
    
    EXPECT_CALL(*simulator1, runSimulation())
        .WillOnce(testing::Invoke(sim1_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator2, runSimulation())
        .WillOnce(testing::Invoke(sim2_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator1, stopSimulation())
        .WillOnce(testing::Invoke(sim1_ptr, &MockSensorSimulator::stopSimulationImpl));
    EXPECT_CALL(*simulator2, stopSimulation())
        .WillOnce(testing::Invoke(sim2_ptr, &MockSensorSimulator::stopSimulationImpl));
    
    manager_->addSimulator(std::move(simulator1));
    manager_->addSimulator(std::move(simulator2));
    
    manager_->startAll();
    
    // Give threads time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_THAT(output, testing::HasSubstr("Starting 2 simulators"));
    EXPECT_THAT(output, testing::HasSubstr("All simulators started"));
}

TEST_F(SimulatorManagerTest, StopAllSimulators)
{
    testing::internal::CaptureStdout();
    
    auto simulator1 = std::make_unique<MockSensorSimulator>();
    auto simulator2 = std::make_unique<MockSensorSimulator>();
    
    auto* sim1_ptr = simulator1.get();
    auto* sim2_ptr = simulator2.get();
    
    EXPECT_CALL(*simulator1, runSimulation())
        .WillOnce(testing::Invoke(sim1_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator2, runSimulation())
        .WillOnce(testing::Invoke(sim2_ptr, &MockSensorSimulator::runSimulationImpl));
    
    EXPECT_CALL(*simulator1, stopSimulation())
        .WillOnce(testing::Invoke(sim1_ptr, &MockSensorSimulator::stopSimulationImpl));
    EXPECT_CALL(*simulator2, stopSimulation())
        .WillOnce(testing::Invoke(sim2_ptr, &MockSensorSimulator::stopSimulationImpl));
    
    manager_->addSimulator(std::move(simulator1));
    manager_->addSimulator(std::move(simulator2));
    
    manager_->startAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    manager_->stopAll();
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_THAT(output, testing::HasSubstr("Stopping all simulators"));
    EXPECT_THAT(output, testing::HasSubstr("All simulators stopped"));
}

TEST_F(SimulatorManagerTest, CannotAddWhileRunning)
{
    testing::internal::CaptureStderr();
    
    auto simulator1 = std::make_unique<MockSensorSimulator>();
    auto* sim1_ptr = simulator1.get();
    
    EXPECT_CALL(*simulator1, runSimulation())
        .WillOnce(testing::Invoke(sim1_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator1, stopSimulation())
        .WillOnce(testing::Invoke(sim1_ptr, &MockSensorSimulator::stopSimulationImpl));
    
    manager_->addSimulator(std::move(simulator1));
    manager_->startAll();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to add another simulator while running
    auto simulator2 = std::make_unique<MockSensorSimulator>();
    manager_->addSimulator(std::move(simulator2));
    
    std::string error = testing::internal::GetCapturedStderr();
    EXPECT_THAT(error, testing::HasSubstr("Cannot add simulator while running"));
}

TEST_F(SimulatorManagerTest, StartAllAlreadyRunning)
{
    testing::internal::CaptureStderr();
    
    auto simulator = std::make_unique<MockSensorSimulator>();
    auto* sim_ptr = simulator.get();
    
    EXPECT_CALL(*simulator, runSimulation())
        .WillOnce(testing::Invoke(sim_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator, stopSimulation())
        .WillOnce(testing::Invoke(sim_ptr, &MockSensorSimulator::stopSimulationImpl));
    
    manager_->addSimulator(std::move(simulator));
    
    manager_->startAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Try to start again
    manager_->startAll();
    
    std::string error = testing::internal::GetCapturedStderr();
    EXPECT_THAT(error, testing::HasSubstr("already running"));
}

TEST_F(SimulatorManagerTest, StopAllWhenNotRunning)
{
    // Should be safe to call stopAll without starting
    manager_->stopAll();
    SUCCEED();
}

TEST_F(SimulatorManagerTest, StopAllMultipleTimes)
{
    auto simulator = std::make_unique<MockSensorSimulator>();
    auto* sim_ptr = simulator.get();
    
    EXPECT_CALL(*simulator, runSimulation())
        .WillOnce(testing::Invoke(sim_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator, stopSimulation())
        .WillOnce(testing::Invoke(sim_ptr, &MockSensorSimulator::stopSimulationImpl));
    
    manager_->addSimulator(std::move(simulator));
    manager_->startAll();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    manager_->stopAll();
    manager_->stopAll(); // Should be safe to call multiple times
    
    SUCCEED();
}

TEST_F(SimulatorManagerTest, DestructorStopsSimulators)
{
    auto simulator = std::make_unique<MockSensorSimulator>();
    auto* sim_ptr = simulator.get();
    
    EXPECT_CALL(*simulator, runSimulation())
        .WillOnce(testing::Invoke(sim_ptr, &MockSensorSimulator::runSimulationImpl));
    EXPECT_CALL(*simulator, stopSimulation())
        .WillOnce(testing::Invoke(sim_ptr, &MockSensorSimulator::stopSimulationImpl));
    
    {
        SensorSimulator::SimulatorManager local_manager;
        local_manager.addSimulator(std::move(simulator));
        local_manager.startAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Destructor called here
    }
    
    SUCCEED();
}

TEST_F(SimulatorManagerTest, SimulatorsRunInSeparateThreads)
{
    std::atomic<int> running_count{0};
    std::atomic<int> max_concurrent{0};
    std::mutex count_mutex;
    
    class CountingSimulator : public SensorSimulator::ISensorSimulator
    {
    public:
        CountingSimulator(std::atomic<int>& running_count, std::atomic<int>& max_concurrent)
            : running_count_(running_count), max_concurrent_(max_concurrent) {}
        
        void runSimulation() override
        {
            int current = ++running_count_;
            
            // Update max concurrent
            int expected = max_concurrent_.load();
            while (current > expected && !max_concurrent_.compare_exchange_weak(expected, current));
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            --running_count_;
        }
        
        void stopSimulation() override
        {
            should_stop_ = true;
        }
        
    private:
        std::atomic<int>& running_count_;
        std::atomic<int>& max_concurrent_;
        std::atomic<bool> should_stop_{false};
    };
    
    manager_->addSimulator(std::make_unique<CountingSimulator>(running_count, max_concurrent));
    manager_->addSimulator(std::make_unique<CountingSimulator>(running_count, max_concurrent));
    manager_->addSimulator(std::make_unique<CountingSimulator>(running_count, max_concurrent));
    
    manager_->startAll();
    
    // Wait for simulators to run
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Should have seen multiple simulators running concurrently
    EXPECT_GE(max_concurrent.load(), 2);
}

TEST_F(SimulatorManagerTest, WithRealSimulators)
{
    EventBus event_bus;
    event_bus.start();
    
    // Use real GasSensorSimulator
    manager_->addSimulator(std::make_unique<SensorSimulator::GasSensorSimulator>(event_bus));
    manager_->addSimulator(std::make_unique<SensorSimulator::TemperatureSensorSimulator>(event_bus));
    
    std::atomic<int> event_count{0};
    event_bus.subscribe([&event_count](const Event::Event& event) {
        event_count++;
    });
    
    manager_->startAll();
    
    // Let simulators run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    manager_->stopAll();
    event_bus.stop();
    
    // Should have received some events
    EXPECT_GT(event_count.load(), 0);
}
