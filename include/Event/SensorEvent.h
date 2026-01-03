#ifndef EVENT_SENSOR_EVENT_H
#define EVENT_SENSOR_EVENT_H

#include <string>
#include <ctime>
#include <cstdint>
#include <random>

#include "Event.h"
#include "Util/RandomNumberGenerator.h"

namespace Event
{

/**
 * @enum Status
 * @brief Status codes for sensor operations
 */
enum class Status {
    OK,    ///< Operation completed successfully
    ERROR  ///< Operation failed
};

/**
 * @enum SensorType
 * @brief Types of sensors supported by the system
 */
enum class SensorType {
    CoSensor,        ///< Carbon monoxide gas sensor (50-150 ppm range)
    TempSensor,      ///< Temperature sensor (15-30°C range)
    PressureSensor   ///< Atmospheric pressure sensor (1013-1033 hPa range)
};

/**
 * @struct SensorEvent
 * @brief Concrete event representing a sensor reading
 * 
 * This class encapsulates data from various sensor types including gas sensors,
 * temperature sensors, and pressure sensors. Each sensor reading includes:
 * - A unique device ID
 * - Timestamp of the measurement
 * - The measured value
 * - Sensor type identification
 * 
 * The class simulates realistic sensor behavior including:
 * - Random variations within expected ranges
 * - Fault simulation (0.0 value with 1% probability)
 * - Type-specific value ranges and characteristics
 */
struct SensorEvent : public Event
{
public:
    /**
     * @brief Constructs a sensor event of the specified type
     * @param type The type of sensor (CoSensor, TempSensor, or PressureSensor)
     * 
     * Initializes the sensor with appropriate default values, generates a device ID,
     * captures the current timestamp, and generates an initial reading.
     */
    explicit SensorEvent(SensorType type) {
        init(type);
    }
    
    /**
     * @brief Default destructor
     */
    ~SensorEvent() = default;

    /**
     * @brief Gets the unique device identifier
     * @return Device ID string in format "<SensorType>_<number>"
     */
    std::string getDeviceId() const {
        return device_id_;
    }

    /**
     * @brief Gets the timestamp of the sensor reading
     * @return Unix timestamp as time_t
     */
    std::time_t getTimestamp() const {
        return timestamp_;
    }

    /**
     * @brief Gets the timestamp as a formatted string
     * @param fmt Format string following strftime conventions
     * @return Formatted timestamp string, or empty string if formatting fails
     * 
     * Default format is "%Y-%m-%d %H:%M:%S" (e.g., "2026-01-03 14:30:45")
     */
    std::string getTimestampString(const char* fmt = "%Y-%m-%d %H:%M:%S") const;

    /**
     * @brief Gets the sensor reading value
     * @return Sensor value in type-specific units:
     *         - CoSensor: parts per million (ppm)
     *         - TempSensor: degrees Celsius
     *         - PressureSensor: hectopascals (hPa)
     *         Returns 0.0 for fault condition (1% probability)
     */
    double getValue() const {
        return value_;
    }

    /**
     * @brief Gets the sensor type
     * @return SensorType enum value
     */
    SensorType getSensorType() const {
        return sensor_type_;
    }

    /**
     * @brief Recalculates sensor value with new reading
     * 
     * Updates the timestamp and generates a new sensor value within the
     * appropriate range for this sensor type. Includes 1% chance of fault.
     */
    void recalc();

    /**
     * @brief Compares sensor events by type
     * @param other Another SensorEvent to compare against
     * @return true if sensor types differ, false if same type
     */
    bool operator!=(const SensorEvent& other) const {
        return !(sensor_type_ == other.getSensorType());
    }

private:
    static constexpr int kSensorIdWidth = 10;   ///< Range for device ID numbering
    static constexpr float kFaultValue = 0.0f;  ///< Value indicating sensor fault

    /**
     * @brief Initializes sensor with type-specific parameters
     * @param type Sensor type to initialize
     */
    void init(SensorType type);
    
    /**
     * @brief Sets type-specific configuration
     * @param type Sensor type
     * @return Status::OK on success, Status::ERROR for unknown type
     */
    Status initType(SensorType type);
    
    /**
     * @brief Captures current system time
     */
    void getTimeStamp() {
        time_t timestamp;
        time(&timestamp);
        timestamp_ = timestamp;
    }

    SensorType sensor_type_;              ///< Type of this sensor
    std::string device_id_{};             ///< Unique device identifier
    std::time_t timestamp_;               ///< Timestamp of measurement
    std::string sensor_type_string_;      ///< String representation of sensor type
    double value_;                        ///< Current sensor reading
    double default_value_;                ///< Base value for this sensor type
    uint32_t uniform_dist_;               ///< Range of variation around base value
    Util::RandomNumberGenerator rand_gen_{std::random_device{}()};  ///< RNG for value generation
};

} // namespace Event
















/*
#include "SensorSimulator/Sensor.h"

namespace Event
{

struct SensorEvent : public Event
{
public:
    explicit SensorEvent(const SensorSimulator::Sensor sensor) : _sensor(std::move(sensor)) {}

    std::string getDeviceId() const {
        return _sensor.getDeviceId();
    }

    std::time_t getTimestamp() const {
        return _sensor.getTimestamp();
    }

    double getValue() const {
        return _sensor.getValue();
    }

    SensorSimulator::SensorType getSensorType() const {
        return _sensor.getSensorType();
    }

    std::string getTimestampString(const char* fmt = "%Y-%m-%d %H:%M:%S") const {
        return _sensor.getTimestampString(fmt);
    }

private:
    SensorSimulator::Sensor _sensor;
};

} // namespace Event
*/

#endif // EVENT_SENSOR_EVENT_H