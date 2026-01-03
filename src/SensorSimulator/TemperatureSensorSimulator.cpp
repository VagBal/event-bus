#include <thread>
#include "SensorSimulator/TemperatureSensorSimulator.h"
#include "Event/SensorEvent.h"

void SensorSimulator::TemperatureSensorSimulator::runSimulation()
{
    std::cout << "TemperatureSensorSimulator started simulation." << "\n";
    Event::SensorEvent temperature_sensor(Event::SensorType::TempSensor);

    while (!stop_requested_.load())
    {
        temperature_sensor.recalc();
        event_bus_.publish(std::make_unique<Event::SensorEvent>(temperature_sensor));
        std::this_thread::sleep_for(std::chrono::seconds(sleep_duration_));
    }
}