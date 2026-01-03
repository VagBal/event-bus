#include <thread>
#include "SensorSimulator/GasSensorSimulator.h"
#include "Event/SensorEvent.h"

void SensorSimulator::GasSensorSimulator::runSimulation()
{
    std::cout << "GasSensorSimulator started simulation." << "\n";
    Event::SensorEvent gas_sensor(Event::SensorType::CoSensor);

    while (!stop_requested_.load())
    {
        gas_sensor.recalc();
        event_bus_.publish(std::make_unique<Event::SensorEvent>(gas_sensor));
        std::this_thread::sleep_for(std::chrono::seconds(sleep_duration_));
    }
}