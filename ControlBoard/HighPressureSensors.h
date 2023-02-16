#ifndef HIGH_PRESSURE_SENSORS_H
#define HIGH_PRESSURE_SENSORS_H
#include "Arduino.h"

class HighPressureSensors {
   public:
    static void load_zero();
    static void set_sample_n(uint16_t n);
    static float cal_zero(uint8_t sensor_i, bool save);
    static void set_zero(uint8_t sensor_i, float zero);
    static float read_pressure(uint8_t sensor_i);

   private:
    static const float c0;
    static const float c1;
    static const float c2;
    static float c3[2];
    static const uint8_t pins[2];
    static uint16_t sample_n;

    static float read_pressure(uint8_t sensor_i, uint16_t n);
    static float read_raw_avg(uint8_t sensor_i, uint16_t n);
};

#endif