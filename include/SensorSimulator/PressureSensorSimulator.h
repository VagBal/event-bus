#ifndef SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H

#include "GenericSimulator.h"

namespace SensorSimulator
{
using PressureSensorSimulator = GenericSimulator<Event::SensorType::PressureSensor, 1>;
} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_PRESSURE_SENSOR_SIMULATOR_H