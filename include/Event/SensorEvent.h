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

enum class Status {
    OK,
    ERROR
};

enum class SensorType {
    CoSensor,
    TempSensor,
    PressureSensor
};

struct SensorEvent : public Event
{
public:
    explicit SensorEvent(SensorType type) {
        init(type);
    }
    ~SensorEvent() = default;

    std::string getDeviceId() const {
        return device_id_;
    }

    std::time_t getTimestamp() const {
        return timestamp_;
    }

    std::string getTimestampString(const char* fmt = "%Y-%m-%d %H:%M:%S") const;

    double getValue() const {
        return value_;
    }

    SensorType getSensorType() const {
        return sensor_type_;
    }

    void recalc();

    bool operator!=(const SensorEvent& other) const {
        return !(sensor_type_ == other.getSensorType());
    }

private:
    static constexpr int kSensorIdWidth = 10;
    static constexpr float kFaultValue = 0.0f;

    void init(SensorType type);
    Status initType(SensorType type);
    
    void getTimeStamp() {
        time_t timestamp;
        time(&timestamp);
        timestamp_ = timestamp;
    }

    SensorType sensor_type_;
    std::string device_id_{};
    std::time_t timestamp_;
    std::string sensor_type_string_;
    double value_;
    double default_value_;
    uint32_t uniform_dist_;
    Util::RandomNumberGenerator rand_gen_{std::random_device{}()};
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