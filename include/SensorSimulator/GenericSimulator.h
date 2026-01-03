#ifndef SENSOR_SIMULATOR_GENERIC_SIMULATOR_H
#define SENSOR_SIMULATOR_GENERIC_SIMULATOR_H

#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

#include "ISensorSimulator.h"
#include "EventBus/EventBus.h"
#include "Event/SensorEvent.h"

namespace SensorSimulator
{

template<Event::SensorType T, uint8_t U>
class GenericSimulator : public ISensorSimulator
{
public:
    explicit GenericSimulator(EventBus& event_bus) 
        : event_bus_(event_bus),
        stop_requested_(false) {}
    
    ~GenericSimulator() override = default;

    void runSimulation() override
    {
        Event::SensorEvent sensor(T);

        while(!stop_requested_.load(std::memory_order_acquire))
        {
            sensor.recalc();
            event_bus_.publish(std::make_unique<Event::SensorEvent>(sensor));
            std::this_thread::sleep_for(std::chrono::seconds(U));
        }
    }

    void stopSimulation() override
    {
        stop_requested_.store(true, std::memory_order_release);
    }
private:
    EventBus& event_bus_;
    std::atomic<bool> stop_requested_;
};

} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_GENERIC_SIMULATOR_H