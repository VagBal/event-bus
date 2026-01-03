#ifndef SENSOR_SIMULATOR_I_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_I_SENSOR_SIMULATOR_H

namespace SensorSimulator
{

/**
 * @interface ISensorSimulator
 * @brief Abstract interface for all sensor simulators
 * 
 * This interface defines the contract that all sensor simulator implementations
 * must follow. It provides basic lifecycle control for starting and stopping
 * sensor data generation.
 * 
 * Implementations should:
 * - Generate sensor readings at appropriate intervals
 * - Publish events to the EventBus
 * - Respond promptly to stopSimulation() calls
 * - Be thread-safe for concurrent start/stop operations
 */
class ISensorSimulator
{
public:
    /**
     * @brief Virtual destructor for proper polymorphic deletion
     */
    virtual ~ISensorSimulator() = default;
    
    /**
     * @brief Starts the sensor simulation loop
     * 
     * This method should block and continuously generate sensor readings
     * until stopSimulation() is called. Typically runs on a dedicated thread.
     * 
     * Implementations should:
     * - Generate periodic sensor events
     * - Publish events to the EventBus
     * - Check stop flag regularly
     * - Return cleanly when stopped
     */
    virtual void runSimulation() = 0;
    
    /**
     * @brief Signals the simulator to stop
     * 
     * Sets a flag that causes runSimulation() to exit. Should be non-blocking
     * and thread-safe. After calling this, runSimulation() should return
     * within a reasonable time (typically within one simulation interval).
     */
    virtual void stopSimulation() = 0;
};

} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_I_SENSOR_SIMULATOR_H