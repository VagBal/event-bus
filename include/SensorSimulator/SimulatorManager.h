#ifndef SENSOR_SIMULATOR_SIMULATOR_MANAGER_H
#define SENSOR_SIMULATOR_SIMULATOR_MANAGER_H

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

#include "ISensorSimulator.h"

namespace SensorSimulator
{

class SimulatorManager
{
enum class SimulatorState
{
    Stopped,
    Running
};

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
    std::vector<std::unique_ptr<ISensorSimulator>> simulators_;
    std::vector<std::thread> threads_;
    std::atomic<SimulatorState> state_{SimulatorState::Stopped};
    mutable std::mutex mutex_;
};

} // namespace SensorSimulator 

#endif // SENSOR_SIMULATOR_SIMULATOR_MANAGER_H