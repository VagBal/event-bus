#ifndef SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H

#include "GenericSimulator.h"

namespace SensorSimulator
{

/**
 * @typedef PressureSensorSimulator
 * @brief Atmospheric pressure sensor simulator with 1-second update interval
 * 
 * Simulates a pressure sensor that generates readings in the 1013-1033 hPa range.
 * Updates every 1 second for high-frequency pressure monitoring.
 * 
 * Sensor characteristics:
 * - Type: PressureSensor
 * - Range: 1013-1033 hectopascals (hPa)
 * - Update interval: 1 second
 * - Fault rate: 1% (returns 0.0 value)
 */
using PressureSensorSimulator = GenericSimulator<Event::SensorType::PressureSensor, 1>;
} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H