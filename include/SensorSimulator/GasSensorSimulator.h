#ifndef SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H

#include <iostream>
#include <cstdint>
#include <atomic>

#include "ISensorSimulator.h"
#include "EventBus/EventBus.h"

namespace SensorSimulator
{

class GasSensorSimulator : public ISensorSimulator
{
public:
    explicit GasSensorSimulator(EventBus& event_bus, uint8_t sleep_duration = 10) : event_bus_(event_bus), sleep_duration_(sleep_duration)
    {
        std::cout << "GasSensorSimulator initialized with: " << static_cast<int>(sleep_duration_) << " seconds sleep!" << "\n";
    }

    ~GasSensorSimulator() = default;

    void runSimulation() override;

    void stopSimulation() override
    {
        std::cout << "GasSensorSimulator stopped simulation." << "\n";
        stop_requested_.store(true);
    }

private:
    EventBus& event_bus_;
    uint8_t sleep_duration_;
    std::atomic<bool> stop_requested_{false};
};

} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H