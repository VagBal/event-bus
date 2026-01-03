#ifndef CONSUMER_SIMULATOR_TEST_CONSUMER_SIMULATOR_H
#define CONSUMER_SIMULATOR_TEST_CONSUMER_SIMULATOR_H

#include <iostream>

#include "EventBus/EventBus.h"
#include "Event/Event.h"
#include "Event/SensorEvent.h"

namespace ConsumerSimulator
{

/**
 * @class TestConsumerSimulator
 * @brief Example consumer that processes and logs sensor events
 * 
 * This class demonstrates a typical event consumer in the system. It subscribes
 * to the EventBus and processes incoming SensorEvents, specifically:
 * - Logs all CO sensor readings with detailed information
 * - Detects and reports sensor faults (zero values)
 * - Filters events by sensor type
 * 
 * The consumer outputs formatted information to stdout for monitoring and debugging.
 * This serves as both a functional consumer and a template for implementing custom
 * event handlers.
 */
class TestConsumerSimulator
{
public:
    /**
     * @brief Constructs the consumer and subscribes to the event bus
     * @param event_bus Reference to the EventBus to subscribe to
     * 
     * Automatically registers an event handler that will be called for all
     * published events. The subscription persists for the lifetime of this object.
     */
    explicit TestConsumerSimulator(EventBus& event_bus)
    {
        std::cout << "TestConsumerSimulator initialized, and subscribed to EventBus." << "\n";
        event_bus.subscribe([this](const Event::Event& event) {
            onEvent(event);
        });
    }

    /**
     * @brief Default destructor
     */
    ~TestConsumerSimulator() = default;
    
private:
    /**
     * @brief Event handler callback for processing incoming events
     * @param event The event to process
     * 
     * Filters for SensorEvents and:
     * 1. Logs CO sensor readings with device ID, timestamp, and value
     * 2. Detects fault conditions (value == 0.0) for any sensor type
     *    and logs warning messages
     */
    void onEvent(const Event::Event& event);
};

} // namespace ConsumerSimulator

#endif // CONSUMER_SIMULATOR_TEST_CONSUMER_SIMULATOR_H