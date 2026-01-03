#ifndef SENSOR_SIMULATOR_SIMULATOR_MANAGER_H
#define SENSOR_SIMULATOR_SIMULATOR_MANAGER_H

#include <iostream>
#include <thread>
#include <vector>
#include <memory>

#include "ISensorSimulator.h"

namespace SensorSimulator
{

class SimulatorManager
{
public:
    SimulatorManager() = default;
    ~SimulatorManager();

    // Delete copy constructor and assignment operator to prevent copying
    SimulatorManager(const SimulatorManager&) = delete;
    SimulatorManager& operator=(const SimulatorManager&) = delete;

    void addSimulator(std::unique_ptr<ISensorSimulator> simulator);
    void startAll();
    void stopAll();
private:
    enum class SimulatorState
    {
        Stopped,
        Running
    };
    std::vector<std::unique_ptr<ISensorSimulator>> simulators_;
    std::vector<std::thread> threads_;
    SimulatorState state_ = SimulatorState::Stopped;
};

} // namespace SensorSimulator 

#endif // SENSOR_SIMULATOR_SIMULATOR_MANAGER_H