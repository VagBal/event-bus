#ifndef SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H

#include <iostream>
#include <cstdint>
#include <atomic>

#include "ISensorSimulator.h"
#include "EventBus/EventBus.h"

namespace SensorSimulator
{

class PressureSensorSimulator : public ISensorSimulator
{
public:
    explicit PressureSensorSimulator(EventBus& event_bus, uint8_t sleep_duration = 2) : event_bus_(event_bus), sleep_duration_(sleep_duration)
    {
        std::cout << "PressureSensorSimulator initialized with: " << static_cast<int>(sleep_duration_) << " seconds sleep!" << "\n";
    }

    ~PressureSensorSimulator() = default;

    void runSimulation() override;

    void stopSimulation() override
    {
        std::cout << "PressureSensorSimulator stopped simulation." << "\n";
        stop_requested_.store(true);
    }
    
private:
    EventBus& event_bus_;
    uint8_t sleep_duration_;
    std::atomic<bool> stop_requested_{false};
};

} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H