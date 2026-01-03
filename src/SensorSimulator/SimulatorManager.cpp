#include "SensorSimulator/SimulatorManager.h"
#include <iostream>

/**
 * @brief Destructor - ensures clean shutdown
 * 
 * Automatically stops all running simulators and joins threads
 * to prevent resource leaks.
 */
SensorSimulator::SimulatorManager::~SimulatorManager()
{
    // Stop all simulators and join threads
    stopAll();
}

/**
 * @brief Adds a simulator to the managed collection
 * @param simulator Unique pointer to simulator (ownership transferred)
 * 
 * Fails with error message if called while simulators are running.
 * Thread-safe for concurrent additions when stopped.
 */
void SensorSimulator::SimulatorManager::addSimulator(std::unique_ptr<ISensorSimulator> simulator)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_.load(std::memory_order_acquire) == SimulatorState::Running)
    {
        std::cerr << "Cannot add simulator while running." << "\n";
        return;
    }
    // Add the given simulator resource tot the list
    simulators_.emplace_back(std::move(simulator));
}

/**
 * @brief Starts all simulators on separate threads
 * 
 * Creates one thread per simulator and calls runSimulation() on each.
 * Uses atomic compare-exchange for thread-safe state transition.
 * Idempotent - logs error if already running.
 */
void SensorSimulator::SimulatorManager::startAll()
{
    SimulatorState expected = SimulatorState::Stopped;
    if (!state_.compare_exchange_strong(expected, SimulatorState::Running,
        std::memory_order_acq_rel))
    {
        std::cerr << "Simulators are already running." << "\n";
        return; // Already running
    }

    std::cout << "Starting " << simulators_.size() << " simulators." << "\n";

    // Start all simulators in separate threads
    for (auto& simulator : simulators_)
    {
        // Capture raw pointer by value to avoid dangling reference to loop variable
        ISensorSimulator * simPtr = simulator.get();
        threads_.emplace_back([simPtr]() {
            simPtr->runSimulation();
        });
    }

    std::cout << "All simulators started." << "\n";
}

/**
 * @brief Stops all simulators and waits for threads
 * 
 * Calls stopSimulation() on each simulator, then joins all threads.
 * Uses atomic compare-exchange for thread-safe state transition.
 * Idempotent - returns immediately if not running.
 */
void SensorSimulator::SimulatorManager::stopAll()
{
    SimulatorState expected = SimulatorState::Running;
    if (!state_.compare_exchange_strong(expected, SimulatorState::Stopped,
        std::memory_order_acq_rel))
    {
        return; // Already stopped
    }

    std::cout << "Stopping all simulators..." << "\n";

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
    std::cout << "All simulators stopped." << "\n";
}

