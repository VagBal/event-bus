#ifndef SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H

#include "GenericSimulator.h"

namespace SensorSimulator
{
using GasSensorSimulator = GenericSimulator<Event::SensorType::CoSensor, 10>;
} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_GAS_SENSOR_SIMULATOR_H