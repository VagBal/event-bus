#ifndef SENSOR_SIMULATOR_I_SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_I_SENSOR_SIMULATOR_H

namespace SensorSimulator
{
class ISensorSimulator
{
public:
    virtual ~ISensorSimulator() = default;
    virtual void runSimulation() = 0;
    virtual void stopSimulation() = 0;
};

} // namespace SensorSimulator

#endif // SENSOR_SIMULATOR_I_SENSOR_SIMULATOR_H