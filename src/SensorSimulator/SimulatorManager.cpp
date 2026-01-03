#include "SensorSimulator/SimulatorManager.h"


SensorSimulator::SimulatorManager::~SimulatorManager()
{
    // Stop all simulators and join threads
    stopAll();
}

void SensorSimulator::SimulatorManager::addSimulator(std::unique_ptr<ISensorSimulator> simulator)
{
    // Add the given simulator resource tot the list
    simulators_.emplace_back(std::move(simulator));
}

void SensorSimulator::SimulatorManager::startAll()
{
    if (state_ == SimulatorState::Running)
    {
        return; // Already running
    }

    state_ = SimulatorState::Running;

    // Start all simulators in separate threads
    for (auto& simulator : simulators_)
    {
        ISensorSimulator * simPtr = simulator.get();
        threads_.emplace_back([simPtr]() {
            simPtr->runSimulation();
        });
    }
}

void SensorSimulator::SimulatorManager::stopAll()
{
    if (state_ == SimulatorState::Stopped)
    {
        return; // Already stopped
    }

    state_ = SimulatorState::Stopped;

    for (const auto& simulator : simulators_)
    {
        simulator->stopSimulation();
    }

    for (auto& thread : threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    threads_.clear();
}

