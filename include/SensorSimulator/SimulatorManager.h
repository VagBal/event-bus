#ifndef SENSOR_SIMULATOR_SIMULATOR_MANAGER_H
#define SENSOR_SIMULATOR_SIMULATOR_MANAGER_H

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

#include "ISensorSimulator.h"

namespace SensorSimulator
{

/**
 * @class SimulatorManager
 * @brief Manages lifecycle and concurrent execution of multiple sensor simulators
 * 
 * This class coordinates multiple sensor simulators, allowing them to:
 * - Run concurrently on separate threads
 * - Start and stop as a group
 * - Be added dynamically before starting
 * 
 * The manager ensures proper resource cleanup and thread synchronization:
 * - Automatically stops all simulators in destructor
 * - Prevents adding simulators while running
 * - Uses atomic operations for thread-safe state management
 * - Joins all threads on shutdown to prevent resource leaks
 * 
 * State Machine:
 * @code
 * [Stopped] --startAll()--> [Running] --stopAll()--> [Stopped]
 *     ^                                                   |
 *     |                                                   |
 *     +---------------------------------------------------+
 * @endcode
 * 
 * Thread Safety: All public methods are thread-safe. However, addSimulator()
 * will fail if called while simulators are running.
 */
class SimulatorManager
{
/**
 * @enum SimulatorState
 * @brief Internal state of the simulator manager
 */
enum class SimulatorState
{
    Stopped,  ///< Simulators are not running
    Running   ///< Simulators are currently executing
};

public:
    /**
     * @brief Default constructor
     */
    SimulatorManager() = default;
    
    /**
     * @brief Destructor - stops all simulators and joins threads
     * 
     * Ensures clean shutdown by calling stopAll() to terminate all
     * simulator threads before destruction.
     */
    ~SimulatorManager();

    /**
     * @brief Deleted copy constructor - SimulatorManager is not copyable
     */
    SimulatorManager(const SimulatorManager&) = delete;
    
    /**
     * @brief Deleted assignment operator - SimulatorManager is not assignable
     */
    SimulatorManager& operator=(const SimulatorManager&) = delete;

    /**
     * @brief Adds a simulator to be managed
     * @param simulator Unique pointer to a simulator (ownership transferred)
     * 
     * The simulator will be started when startAll() is called. This method
     * fails and logs an error if called while simulators are running.
     * 
     * @warning Cannot add simulators while in Running state
     * Thread Safety: Can be called from any thread when not running
     */
    void addSimulator(std::unique_ptr<ISensorSimulator> simulator);
    
    /**
     * @brief Starts all managed simulators concurrently
     * 
     * Creates a dedicated thread for each simulator and calls runSimulation()
     * on each. If already running, logs an error and returns without action.
     * 
     * Each simulator runs independently on its own thread until stopAll()
     * is called or the manager is destroyed.
     * 
     * Thread Safety: Can be called from any thread
     * @note Idempotent - safe to call multiple times (only first call has effect)
     */
    void startAll();
    
    /**
     * @brief Stops all simulators and waits for threads to complete
     * 
     * Calls stopSimulation() on each managed simulator, then joins all
     * threads to ensure clean shutdown. If not running, returns immediately.
     * 
     * After this call completes:
     * - All simulator threads have terminated
     * - Manager is back in Stopped state
     * - New simulators can be added
     * - startAll() can be called again
     * 
     * Thread Safety: Can be called from any thread
     * @note Idempotent - safe to call multiple times (only first call has effect)
     */
    void stopAll();
    
private:
    std::vector<std::unique_ptr<ISensorSimulator>> simulators_;  ///< Owned simulator instances
    std::vector<std::thread> threads_;                           ///< Worker threads for simulators
    std::atomic<SimulatorState> state_{SimulatorState::Stopped}; ///< Current state (atomic for thread safety)
    mutable std::mutex mutex_;                                   ///< Protects simulator vector during add
};

} // namespace SensorSimulator 

#endif // SENSOR_SIMULATOR_SIMULATOR_MANAGER_H