#include <thread>
#include "ConsumerSimulator/TestConsumerSimulator.h"
#include "Event/SensorEvent.h"

/**
 * @brief Processes incoming events and logs sensor data
 * @param event The event to process
 * 
 * Filters for SensorEvents and:
 * 1. Logs CO sensor readings with device ID, timestamp, and value
 * 2. Detects and reports fault conditions (value == 0.0)
 * 
 * Non-CO sensor events are silently ignored. Fault events are
 * reported for all sensor types.
 */
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