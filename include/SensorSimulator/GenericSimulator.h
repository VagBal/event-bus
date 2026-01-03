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

/**
 * @class GenericSimulator
 * @brief Template-based sensor simulator for different sensor types and intervals
 * 
 * This template class provides a generic implementation of ISensorSimulator that can
 * simulate any sensor type with configurable update intervals. It continuously generates
 * sensor readings and publishes them to the EventBus.
 * 
 * The template parameters allow compile-time configuration of:
 * - Which sensor type to simulate (CO, Temperature, Pressure)
 * - How frequently to generate readings (in seconds)
 * 
 * This design enables type-safe sensor simulators with zero runtime overhead for
 * configuration. All sensor logic is handled by the SensorEvent class.
 * 
 * @tparam T Sensor type from Event::SensorType enum
 * @tparam U Update interval in seconds (how often to generate readings)
 * 
 * Example usage:
 * @code
 * // Create a temperature simulator that updates every 5 seconds
 * GenericSimulator<Event::SensorType::TempSensor, 5> temp_sim(event_bus);
 * @endcode
 * 
 * Thread Safety: stopSimulation() can be called from any thread while
 * runSimulation() is executing.
 */
template<Event::SensorType T, uint8_t U>
class GenericSimulator : public ISensorSimulator
{
public:
    /**
     * @brief Constructs a sensor simulator
     * @param event_bus Reference to the EventBus where events will be published
     * 
     * The simulator is initially stopped and must be started by calling runSimulation().
     */
    explicit GenericSimulator(EventBus& event_bus) 
        : event_bus_(event_bus),
        stop_requested_(false) {}
    
    /**
     * @brief Default destructor
     */
    ~GenericSimulator() override = default;

    /**
     * @brief Runs the sensor simulation loop
     * 
     * Continuously generates sensor readings at the configured interval (U seconds)
     * and publishes them to the EventBus. This method blocks until stopSimulation()
     * is called.
     * 
     * The simulation loop:
     * 1. Creates a SensorEvent of type T
     * 2. Recalculates the sensor value (generates new reading)
     * 3. Publishes the event to the EventBus
     * 4. Sleeps for U seconds
     * 5. Checks if stop was requested
     * 6. Repeats until stopped
     * 
     * Thread Safety: Safe to call stopSimulation() from another thread
     */
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

    /**
     * @brief Signals the simulation to stop
     * 
     * Sets an atomic flag that causes runSimulation() to exit after completing
     * the current sleep interval. This method returns immediately and is safe
     * to call from any thread.
     * 
     * Thread Safety: Can be called from any thread concurrently with runSimulation()
     */
    void stopSimulation() override
    {
        stop_requested_.store(true, std::memory_order_release);
    }
    
private:
    EventBus& event_bus_;               ///< Reference to the event publishing system
    std::atomic<bool> stop_requested_;  ///< Flag to signal simulation stop
};

} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_GENERIC_SIMULATOR_H