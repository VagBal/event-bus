#include <cassert>
#include <iostream>

#include "Event/SensorEvent.h"

/**
 * @brief Initializes sensor with type-specific configuration and initial value
 * @param type The sensor type to initialize
 * 
 * Performs complete sensor initialization:
 * 1. Captures current timestamp
 * 2. Configures type-specific parameters (range, base value, etc.)
 * 3. Generates unique device ID
 * 4. Generates initial sensor reading with 1% fault probability
 */
void Event::SensorEvent::init(SensorType type) {
    // Get the timestamp for the current date and time
    getTimeStamp();

    Status status = initType(type);

    if (status == Status::ERROR) {
        assert(false && "Failed to initialize sensor type");
        return;
    }

    if (device_id_.empty()) {
        device_id_ = sensor_type_string_ + std::to_string(rand_gen_.uniform_dist(kSensorIdWidth));
    }

    if (rand_gen_.one_in(100)) {
        value_ = kFaultValue; // Simulate a faulty reading with 1% probability
    } else {
        value_ = default_value_ + static_cast<double>(rand_gen_.uniform_dist(uniform_dist_));
    }
}

/**
 * @brief Generates a new sensor reading with updated timestamp
 * 
 * Updates the timestamp to current time and generates a new random value
 * within the sensor's configured range. Includes 1% probability of fault
 * condition (0.0 value).
 */
void Event::SensorEvent::recalc() {
    getTimeStamp();
    
    if (rand_gen_.one_in(100)) {
        value_ = kFaultValue; // Simulate a faulty reading with 1% probability
    } else {
        value_ = default_value_ + static_cast<double>(rand_gen_.uniform_dist(uniform_dist_));
    }
}


/**
 * @brief Configures sensor parameters based on type
 * @param type Sensor type to configure
 * @return Status::OK on success, Status::ERROR for unknown type
 * 
 * Sets type-specific parameters:
 * - CoSensor: 50-150 ppm range
 * - TempSensor: 15-30°C range  
 * - PressureSensor: 1013-1033 hPa range
 */
Event::Status Event::SensorEvent::initType(SensorType type)
{
    switch (type) {
        case SensorType::CoSensor:
            sensor_type_ = SensorType::CoSensor;
            sensor_type_string_ = "CoSensor_";
            default_value_ = 50.0;
            uniform_dist_ = 100;
            break;
        case SensorType::TempSensor:
            sensor_type_ = SensorType::TempSensor;
            sensor_type_string_ = "TempSensor_";
            default_value_ = 15.0;
            uniform_dist_ = 15;
            break;
        case SensorType::PressureSensor:
            sensor_type_ = SensorType::PressureSensor;
            sensor_type_string_ = "PressureSensor_";
            default_value_ = 1013.25;
            uniform_dist_ = 20;
            break;
        default:
            std::cout << "Unknown sensor type!" << "\n";
            return Status::ERROR;
    }
    return Status::OK;
}

/**
 * @brief Formats timestamp as string using specified format
 * @param fmt strftime format string
 * @return Formatted timestamp, or empty string on error
 * 
 * Uses platform-specific localtime functions for thread safety.
 * Default format: "%Y-%m-%d %H:%M:%S"
 */
std::string Event::SensorEvent::getTimestampString(const char* fmt) const {
    std::time_t time = timestamp_;
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif
    char buffer[64];
    if (std::strftime(buffer, sizeof(buffer), fmt, &tm_buf) == 0) {
        return std::string();
    }
    return std::string(buffer);
}