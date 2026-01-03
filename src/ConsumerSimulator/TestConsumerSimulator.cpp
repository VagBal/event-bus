#include <thread>
#include "ConsumerSimulator/TestConsumerSimulator.h"
#include "Event/SensorEvent.h"

void ConsumerSimulator::TestConsumerSimulator::onEvent(const Event::Event& event)
{
    if (const auto* sensor_event = dynamic_cast<const Event::SensorEvent*>(&event)) {
        // Process the sensor event
        if (sensor_event->getSensorType() == Event::SensorType::CoSensor) {
            std::cout << "----------------------------------------" << "\n";
            std::cout << "Processing SensorEvent in TestConsumerSimulator." << "\n";
            std::cout << "Device ID: " << sensor_event->getDeviceId() << "\n";
            std::cout << "Timestamp: " << sensor_event->getTimestampString() << "\n";
            std::cout << "Value: " << sensor_event->getValue() << "\n";
            std::cout << "----------------------------------------" << "\r\n";
        }

        if (!sensor_event->getValue()) {
            std::cout << "----------------------------------------" << "\n";
            std::cout << "THERE WAS A FAILURE IN THIS SENSOR." << "\n";
            std::cout << "Device ID: " << sensor_event->getDeviceId() << "\n";
            std::cout << "Timestamp: " << sensor_event->getTimestampString() << "\n";
            std::cout << "Value: " << sensor_event->getValue() << "\n";
            std::cout << "----------------------------------------" << "\r\n";
        }
    }
}