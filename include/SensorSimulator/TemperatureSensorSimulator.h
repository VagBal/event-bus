#ifndef SENSOR_SIMULATOR_TEMPERATURE_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_TEMPERATURE_SENSOR_SIMULATOR_H

#include "GenericSimulator.h"
namespace SensorSimulator
{
using TemperatureSensorSimulator = GenericSimulator<Event::SensorType::TempSensor, 5>;
} // namespace SensorSimulator
#endif // SENSOR_SIMULATOR_TEMPERATURE_SENSOR_SIMULATOR_H