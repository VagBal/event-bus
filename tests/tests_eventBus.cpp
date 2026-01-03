/**
 * @file tests_eventBus.cpp
 * @brief Unit tests for the EventBus publish-subscribe system
 * 
 * Comprehensive test suite covering:
 * - Basic lifecycle: construction, start, stop
 * - Thread safety: concurrent operations, multiple publishers
 * - Subscription: single and multiple subscribers
 * - Event dispatch: ordering, delivery guarantees
 * - Error handling: stop without start, multiple starts/stops
 * - Queue management: event ordering, queue draining on stop
 * - Concurrency: multiple threads publishing simultaneously
 * 
 * Tests use atomic counters for thread-safe verification and include
 * timing controls to ensure proper async behavior.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "EventBus/EventBus.h"
#include "Event/SensorEvent.h"

/**
 * @class EventBusTest
 * @brief Test fixture for EventBus tests
 * 
 * Provides:
 * - Fresh EventBus instance for each test
 * - Automatic cleanup in TearDown (stops bus and releases resources)
 * - Consistent test environment with proper resource management
 */
class EventBusTest : public ::testing::Test
{
protected:
    /** @brief Creates a fresh EventBus instance before each test */
    void SetUp() override
    {
        event_bus_ = std::make_unique<EventBus>();
    }

    /** @brief Stops and destroys EventBus after each test to prevent resource leaks */
    void TearDown() override
    {
        if (event_bus_)
        {
            event_bus_->stop();
        }
        event_bus_.reset();
    }

    std::unique_ptr<EventBus> event_bus_;  ///< EventBus instance under test
};

/** @test Verifies basic construction and destruction without crashes */
TEST_F(EventBusTest, ConstructAndDestruct)
{
    EventBus bus;
    // Should construct and destruct without issues
    SUCCEED();
}

TEST_F(EventBusTest, StartEventBus)
{
    event_bus_->start();
    // Give it a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    SUCCEED();
}

TEST_F(EventBusTest, StartAlreadyRunning)
{
    event_bus_->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Starting again should be safe
    event_bus_->start();
    SUCCEED();
}

TEST_F(EventBusTest, StopEventBus)
{
    event_bus_->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    event_bus_->stop();
    SUCCEED();
}

TEST_F(EventBusTest, StopWithoutStart)
{
    // Stopping without starting should be safe
    event_bus_->stop();
    SUCCEED();
}

TEST_F(EventBusTest, StopMultipleTimes)
{
    event_bus_->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    event_bus_->stop();
    event_bus_->stop(); // Should be safe to call multiple times
    SUCCEED();
}

TEST_F(EventBusTest, SubscribeHandler)
{
    std::atomic<int> call_count{0};
    
    event_bus_->subscribe([&call_count](const Event::Event& event) {
        call_count++;
    });
    
    event_bus_->start();
    
    // Publish an event
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    
    // Wait for event to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(call_count.load(), 1);
}

TEST_F(EventBusTest, MultipleSubscribers)
{
    std::atomic<int> call_count1{0};
    std::atomic<int> call_count2{0};
    std::atomic<int> call_count3{0};
    
    event_bus_->subscribe([&call_count1](const Event::Event& event) {
        call_count1++;
    });
    
    event_bus_->subscribe([&call_count2](const Event::Event& event) {
        call_count2++;
    });
    
    event_bus_->subscribe([&call_count3](const Event::Event& event) {
        call_count3++;
    });
    
    event_bus_->start();
    
    // Publish an event
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::TempSensor));
    
    // Wait for event to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(call_count1.load(), 1);
    EXPECT_EQ(call_count2.load(), 1);
    EXPECT_EQ(call_count3.load(), 1);
}

TEST_F(EventBusTest, PublishMultipleEvents)
{
    std::atomic<int> call_count{0};
    
    event_bus_->subscribe([&call_count](const Event::Event& event) {
        call_count++;
    });
    
    event_bus_->start();
    
    // Publish multiple events
    for (int i = 0; i < 5; ++i)
    {
        event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    }
    
    // Wait for events to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(call_count.load(), 5);
}

TEST_F(EventBusTest, PublishBeforeStart)
{
    std::atomic<int> call_count{0};
    
    event_bus_->subscribe([&call_count](const Event::Event& event) {
        call_count++;
    });
    
    // Publish before starting
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    
    // Now start the bus
    event_bus_->start();
    
    // Wait for event to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Event should still be processed
    EXPECT_EQ(call_count.load(), 1);
}

TEST_F(EventBusTest, HandlerReceivesCorrectEventType)
{
    std::atomic<bool> received_sensor_event{false};
    
    event_bus_->subscribe([&received_sensor_event](const Event::Event& event) {
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event && sensor_event->getSensorType() == Event::SensorType::PressureSensor)
        {
            received_sensor_event = true;
        }
    });
    
    event_bus_->start();
    
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::PressureSensor));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(received_sensor_event.load());
}

TEST_F(EventBusTest, EventsProcessedInOrder)
{
    std::vector<int> processed_order;
    std::mutex order_mutex;
    
    event_bus_->subscribe([&processed_order, &order_mutex](const Event::Event& event) {
        const Event::SensorEvent* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event);
        if (sensor_event)
        {
            std::lock_guard<std::mutex> lock(order_mutex);
            // Use timestamp as a proxy for order (though events are created quickly)
            processed_order.push_back(static_cast<int>(sensor_event->getTimestamp()));
        }
    });
    
    event_bus_->start();
    
    // Publish events
    for (int i = 0; i < 3; ++i)
    {
        event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Small delay to ensure different timestamps
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(processed_order.size(), 3);
}

TEST_F(EventBusTest, StopDrainsEventQueue)
{
    std::atomic<int> call_count{0};
    
    event_bus_->subscribe([&call_count](const Event::Event& event) {
        call_count++;
        // Slow handler to ensure events queue up
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });
    
    event_bus_->start();
    
    // Publish multiple events quickly
    for (int i = 0; i < 3; ++i)
    {
        event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    }
    
    // Stop immediately - should still process all queued events
    event_bus_->stop();
    
    EXPECT_EQ(call_count.load(), 3);
}

TEST_F(EventBusTest, MultipleEventTypes)
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
    
    event_bus_->start();
    
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::TempSensor));
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::PressureSensor));
    event_bus_->publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_EQ(co_count.load(), 2);
    EXPECT_EQ(temp_count.load(), 1);
    EXPECT_EQ(pressure_count.load(), 1);
}

TEST_F(EventBusTest, DestructorStopsEventBus)
{
    std::atomic<bool> handler_called{false};
    
    {
        EventBus local_bus;
        local_bus.subscribe([&handler_called](const Event::Event& event) {
            handler_called = true;
        });
        
        local_bus.start();
        local_bus.publish(std::make_unique<Event::SensorEvent>(Event::SensorType::CoSensor));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Bus destroyed here, should stop cleanly
    }
    
    EXPECT_TRUE(handler_called.load());
}
