/**
 * @file tests_sensorEvent.cpp
 * @brief Unit tests for the SensorEvent class
 * 
 * Comprehensive test suite covering:
 * - Construction of all sensor types (CO, Temperature, Pressure)
 * - Device ID generation and formatting
 * - Timestamp handling (Unix time and formatted strings)
 * - Value ranges for each sensor type
 * - recalc() method for updating sensor readings
 * - Fault simulation (0.0 value with ~1% probability)
 * - Operator overloads (inequality comparison)
 * 
 * Tests validate that sensor events maintain correct type-specific characteristics
 * and ranges throughout their lifecycle.
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "Event/Event.h"
#include "Event/SensorEvent.h"

/**
 * @class SensorEventTest
 * @brief Test fixture for SensorEvent tests
 * 
 * Provides a clean test environment for each SensorEvent test case.
 * Currently implements empty SetUp/TearDown for future extension.
 */
class SensorEventTest : public ::testing::Test
{
protected:
    /** @brief Sets up test fixture (currently empty, reserved for future use) */
    void SetUp() override
    {
        // Setup code if needed
    }

    /** @brief Cleans up test fixture (currently empty, reserved for future use) */
    void TearDown() override
    {
        // Cleanup code if needed
    }
};

/** @test Validates CO sensor event creation with correct type and value range (50-150 ppm) */
TEST_F(SensorEventTest, CreateCoSensorEvent)
{
    Event::SensorEvent event(Event::SensorType::CoSensor);
    
    EXPECT_EQ(event.getSensorType(), Event::SensorType::CoSensor);
    EXPECT_FALSE(event.getDeviceId().empty());
    EXPECT_GT(event.getTimestamp(), 0);
    EXPECT_GE(event.getValue(), 0.0);
    EXPECT_LE(event.getValue(), 150.0); // 50 + 100
}

TEST_F(SensorEventTest, CreateTempSensorEvent)
{
    Event::SensorEvent event(Event::SensorType::TempSensor);
    
    EXPECT_EQ(event.getSensorType(), Event::SensorType::TempSensor);
    EXPECT_FALSE(event.getDeviceId().empty());
    EXPECT_GT(event.getTimestamp(), 0);
    EXPECT_GE(event.getValue(), 0.0);
    EXPECT_LE(event.getValue(), 30.0); // 15 + 15
}

TEST_F(SensorEventTest, CreatePressureSensorEvent)
{
    Event::SensorEvent event(Event::SensorType::PressureSensor);
    
    EXPECT_EQ(event.getSensorType(), Event::SensorType::PressureSensor);
    EXPECT_FALSE(event.getDeviceId().empty());
    EXPECT_GT(event.getTimestamp(), 0);
    EXPECT_GE(event.getValue(), 0.0);
    EXPECT_LE(event.getValue(), 1035.0); // 1013.25 + 20
}

TEST_F(SensorEventTest, DeviceIdFormat)
{
    Event::SensorEvent event(Event::SensorType::CoSensor);
    std::string device_id = event.getDeviceId();
    
    EXPECT_TRUE(device_id.find("CoSensor_") != std::string::npos);
}

TEST_F(SensorEventTest, TimestampStringDefaultFormat)
{
    Event::SensorEvent event(Event::SensorType::CoSensor);
    std::string timestamp_str = event.getTimestampString();
    
    EXPECT_FALSE(timestamp_str.empty());
    // Check format YYYY-MM-DD HH:MM:SS (length should be 19)
    EXPECT_EQ(timestamp_str.length(), 19);
}

TEST_F(SensorEventTest, TimestampStringCustomFormat)
{
    Event::SensorEvent event(Event::SensorType::CoSensor);
    std::string timestamp_str = event.getTimestampString("%Y-%m-%d");
    
    EXPECT_FALSE(timestamp_str.empty());
    // Check format YYYY-MM-DD (length should be 10)
    EXPECT_EQ(timestamp_str.length(), 10);
}

TEST_F(SensorEventTest, RecalcUpdatesValue)
{
    Event::SensorEvent event(Event::SensorType::CoSensor);
    
    double initial_value = event.getValue();
    std::time_t initial_timestamp = event.getTimestamp();
    
    // Wait a bit to ensure timestamp changes
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    event.recalc();
    
    // Timestamp should be updated
    EXPECT_GE(event.getTimestamp(), initial_timestamp);
    
    // Value might be the same or different (due to randomness)
    // but should still be in valid range
    EXPECT_GE(event.getValue(), 0.0);
    EXPECT_LE(event.getValue(), 150.0);
}

TEST_F(SensorEventTest, RecalcMultipleTimes)
{
    Event::SensorEvent event(Event::SensorType::TempSensor);
    
    for (int i = 0; i < 10; ++i)
    {
        event.recalc();
        EXPECT_GE(event.getValue(), 0.0);
        EXPECT_LE(event.getValue(), 30.0);
    }
}

TEST_F(SensorEventTest, NotEqualOperatorSameType)
{
    Event::SensorEvent event1(Event::SensorType::CoSensor);
    Event::SensorEvent event2(Event::SensorType::CoSensor);
    
    EXPECT_FALSE(event1 != event2);
}

TEST_F(SensorEventTest, NotEqualOperatorDifferentType)
{
    Event::SensorEvent event1(Event::SensorType::CoSensor);
    Event::SensorEvent event2(Event::SensorType::TempSensor);
    
    EXPECT_TRUE(event1 != event2);
}

TEST_F(SensorEventTest, FaultValueCanOccur)
{
    // Test that fault values (0.0) can occur
    // Run many iterations to increase chance of hitting the 1% fault case
    bool fault_occurred = false;
    
    for (int i = 0; i < 1000; ++i)
    {
        Event::SensorEvent event(Event::SensorType::CoSensor);
        if (event.getValue() == 0.0)
        {
            fault_occurred = true;
            break;
        }
    }
    
    // This test might occasionally fail due to randomness, but with 1000 iterations
    // and 1% probability, it's extremely likely to see at least one fault
    // (probability of not seeing any fault: (0.99)^1000 ≈ 0.00004)
    EXPECT_TRUE(fault_occurred);
}

TEST_F(SensorEventTest, RecalcFaultValueCanOccur)
{
    Event::SensorEvent event(Event::SensorType::TempSensor);
    bool fault_occurred = false;
    
    for (int i = 0; i < 1000; ++i)
    {
        event.recalc();
        if (event.getValue() == 0.0)
        {
            fault_occurred = true;
            break;
        }
    }
    
    EXPECT_TRUE(fault_occurred);
}

TEST_F(SensorEventTest, BaseEventPointer)
{
    Event::SensorEvent sensor_event(Event::SensorType::CoSensor);
    Event::Event* base_ptr = &sensor_event;
    
    // Should be able to use as base Event pointer
    EXPECT_NE(base_ptr, nullptr);
    
    // Should be able to dynamic_cast back
    Event::SensorEvent* derived_ptr = dynamic_cast<Event::SensorEvent*>(base_ptr);
    EXPECT_NE(derived_ptr, nullptr);
}

TEST_F(SensorEventTest, MultipleEventsHaveDifferentDeviceIds)
{
    // Create multiple events and check they have different IDs
    std::vector<std::string> device_ids;
    
    for (int i = 0; i < 20; ++i)
    {
        Event::SensorEvent event(Event::SensorType::CoSensor);
        device_ids.push_back(event.getDeviceId());
    }
    
    // Check that we have at least some different IDs (due to randomness)
    std::sort(device_ids.begin(), device_ids.end());
    auto unique_end = std::unique(device_ids.begin(), device_ids.end());
    size_t unique_count = std::distance(device_ids.begin(), unique_end);
    
    EXPECT_GT(unique_count, 1);
}
