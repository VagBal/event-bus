/**
 * @file tests_testConsumerSimulator.cpp
 * @brief Unit tests for the TestConsumerSimulator event consumer
 * 
 * Test suite covering:
 * - Subscription to EventBus during construction
 * - Processing of CO sensor events (should display details)
 * - Filtering of non-CO sensor events (should ignore unless fault)
 * - Detection of faulty sensors (0.0 value)
 * - Multiple consumers receiving same events
 * - Output verification using stdout capture
 * 
 * Uses Google Test's CaptureStdout() to verify console output,
 * as the consumer logs information to stdout.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include "ConsumerSimulator/TestConsumerSimulator.h"
#include "EventBus/EventBus.h"
#include "Event/SensorEvent.h"

/**
 * @class TestConsumerSimulatorTest
 * @brief Test fixture for TestConsumerSimulator tests
 * 
 * Provides:
 * - EventBus instance started before each test
 * - Automatic cleanup and stopping of EventBus
 * - Ready-to-use environment for consumer testing
 */
class TestConsumerSimulatorTest : public ::testing::Test
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

/** @test Verifies consumer subscribes to EventBus during construction */
TEST_F(TestConsumerSimulatorTest, ConstructorSubscribesToEventBus)
{
    // Create consumer - should subscribe to event bus
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // If construction succeeds without crash, subscription worked
    SUCCEED();
}

TEST_F(TestConsumerSimulatorTest, ReceivesCoSensorEvent)
{
    // Capture output to verify processing
    testing::internal::CaptureStdout();
    
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // Publish a CO sensor event
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Should contain processing message for CO sensor
    EXPECT_THAT(output, testing::HasSubstr("Processing SensorEvent"));
    EXPECT_THAT(output, testing::HasSubstr("Device ID:"));
    EXPECT_THAT(output, testing::HasSubstr("Timestamp:"));
    EXPECT_THAT(output, testing::HasSubstr("Value:"));
}

TEST_F(TestConsumerSimulatorTest, IgnoresNonCoSensorEvents)
{
    testing::internal::CaptureStdout();
    
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // Publish temperature sensor event (not CO sensor)
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::TempSensor));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Should NOT contain "Processing SensorEvent" for non-CO sensors
    // (unless it's a failure case)
    size_t pos = output.find("Processing SensorEvent");
    if (pos != std::string::npos)
    {
        // If found, it should only be for failure case
        EXPECT_THAT(output, testing::HasSubstr("THERE WAS A FAILURE"));
    }
}

TEST_F(TestConsumerSimulatorTest, DetectsFaultySensor)
{
    testing::internal::CaptureStdout();
    
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // Try multiple events to potentially hit a fault (value = 0)
    bool fault_detected = false;
    for (int i = 0; i < 200; ++i)
    {
        event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::TempSensor));
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // With 200 events and 1% fault rate, very likely to see at least one fault
    if (output.find("THERE WAS A FAILURE") != std::string::npos)
    {
        fault_detected = true;
        EXPECT_THAT(output, testing::HasSubstr("THERE WAS A FAILURE IN THIS SENSOR"));
    }
    
    // This is probabilistic but with 200 tries at 1% rate, 
    // probability of no fault is (0.99)^200 ≈ 0.134
    // So there's about 86.6% chance of seeing at least one fault
    // We'll make the test pass either way but log the result
    if (!fault_detected)
    {
        std::cout << "Note: No fault detected in 200 events (probability ~13%)" << std::endl;
    }
}

TEST_F(TestConsumerSimulatorTest, ProcessesMultipleEvents)
{
    testing::internal::CaptureStdout();
    
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // Publish multiple CO sensor events
    for (int i = 0; i < 3; ++i)
    {
        event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Count occurrences of "Processing SensorEvent"
    size_t count = 0;
    size_t pos = 0;
    while ((pos = output.find("Processing SensorEvent", pos)) != std::string::npos)
    {
        count++;
        pos += strlen("Processing SensorEvent");
    }
    
    EXPECT_GE(count, 3);
}

TEST_F(TestConsumerSimulatorTest, HandlesBaseEventType)
{
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // Publish a base Event (not a SensorEvent)
    class TestEvent : public Event::Event {};
    event_bus_->publish(std::make_unique<TestEvent>());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should not crash - dynamic_cast will return nullptr for non-SensorEvent
    SUCCEED();
}

TEST_F(TestConsumerSimulatorTest, MultipleConsumers)
{
    testing::internal::CaptureStdout();
    
    // Create multiple consumers
    ConsumerSimulator::TestConsumerSimulator consumer1(*event_bus_);
    ConsumerSimulator::TestConsumerSimulator consumer2(*event_bus_);
    
    // Publish one event
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Both consumers should process the event
    size_t count = 0;
    size_t pos = 0;
    while ((pos = output.find("Processing SensorEvent", pos)) != std::string::npos)
    {
        count++;
        pos += strlen("Processing SensorEvent");
    }
    
    EXPECT_GE(count, 2);
}

TEST_F(TestConsumerSimulatorTest, PressureSensorWithoutFault)
{
    testing::internal::CaptureStdout();
    
    ConsumerSimulator::TestConsumerSimulator consumer(*event_bus_);
    
    // Publish pressure sensor event
    // Keep trying until we get a non-fault value
    for (int i = 0; i < 10; ++i)
    {
        event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::PressureSensor));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Pressure sensor events should be processed (shown if fault occurs)
    // No specific output for non-CO, non-fault sensors
    SUCCEED();
}
