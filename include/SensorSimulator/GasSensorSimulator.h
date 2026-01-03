#ifndef SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H

#include "GenericSimulator.h"

namespace SensorSimulator
{

/**
 * @typedef GasSensorSimulator
 * @brief Carbon monoxide (CO) gas sensor simulator with 10-second update interval
 * 
 * Simulates a CO sensor that generates readings in the 50-150 ppm range.
 * Updates every 10 seconds to match typical gas sensor sampling rates.
 * 
 * Sensor characteristics:
 * - Type: CoSensor
 * - Range: 50-150 parts per million (ppm)
 * - Update interval: 10 seconds
 * - Fault rate: 1% (returns 0.0 value)
 */
using GasSensorSimulator = GenericSimulator<Event::SensorType::CoSensor, 10>;
} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H