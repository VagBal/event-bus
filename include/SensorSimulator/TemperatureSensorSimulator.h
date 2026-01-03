#ifndef SENSOR_SIMULATOR_TEMPERATURE_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_TEMPERATURE_SENSOR_SIMULATOR_H

#include "GenericSimulator.h"

namespace SensorSimulator
{

/**
 * @typedef TemperatureSensorSimulator  
 * @brief Temperature sensor simulator with 5-second update interval
 * 
 * Simulates a temperature sensor that generates readings in the 15-30°C range.
 * Updates every 5 seconds to match typical temperature sensor sampling rates.
 * 
 * Sensor characteristics:
 * - Type: TempSensor
 * - Range: 15-30 degrees Celsius
 * - Update interval: 5 seconds
 * - Fault rate: 1% (returns 0.0 value)
 */
using TemperatureSensorSimulator = GenericSimulator<Event::SensorType::TempSensor, 5>;
} // namespace SensorSimulator
#endif // SENSOR_SIMULATOR_TEMPERATURE_SENSOR_SIMULATOR_H