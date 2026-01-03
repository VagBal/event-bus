#include <thread>
#include "SensorSimulator/PressureSensorSimulator.h"
#include "Event/SensorEvent.h"

void SensorSimulator::PressureSensorSimulator::runSimulation()
{
    std::cout << "PressureSensorSimulator started simulation." << "\n";
    Event::SensorEvent pressure_sensor(Event::SensorType::PressureSensor);

    while (!stop_requested_.load())
    {
        pressure_sensor.recalc();
        event_bus_.publish(std::make_unique<Event::SensorEvent>(pressure_sensor));
        std::this_thread::sleep_for(std::chrono::seconds(sleep_duration_));
    }
}