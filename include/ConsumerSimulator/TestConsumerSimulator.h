#ifndef CONSUMER_SIMULATOR_TEST_CONSUMER_SIMULATOR_H
#define CONSUMER_SIMULATOR_TEST_CONSUMER_SIMULATOR_H

#include <iostream>

#include "EventBus/EventBus.h"
#include "Event/Event.h"
#include "Event/SensorEvent.h"

namespace ConsumerSimulator
{

class TestConsumerSimulator
{
public:
    explicit TestConsumerSimulator(EventBus& event_bus)
    {
        std::cout << "TestConsumerSimulator initialized, and subscribed to EventBus." << "\n";
        event_bus.subscribe([this](const Event::Event& event) {
            onEvent(event);
        });
    }

    ~TestConsumerSimulator() = default;
private:
    void onEvent(const Event::Event& event);
};

} // namespace ConsumerSimulator

#endif // CONSUMER_SIMULATOR_TEST_CONSUMER_SIMULATOR_H