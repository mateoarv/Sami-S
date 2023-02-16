#ifndef STORAGE_H
#define STORAGE_H
#include "Arduino.h"
#include "flow_sensors.h"
#include "GasSensors.h"

class Storage {
   public:
    static void write_flow_cal(uint8_t sensor_i, const sensor_cal_t& cal);
    static void read_flow_cal(uint8_t sensor_i, sensor_cal_t& cal);
    static void write_p_zero(uint8_t sensor_i, float zero);
    static float read_p_zero(uint8_t sensor_i);
    static void write_gas_p_cal(uint8_t sensor_i, const gas_p_cal_t& cal);
    static void read_gas_p_cal(uint8_t sensor_i, gas_p_cal_t& cal);
    static bool is_blank();
};

#endif