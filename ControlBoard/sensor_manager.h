#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include "Arduino.h"
#include "GasSensors.h"
#include "buff_circ.h"
#include "sensor_buff.h"
#include "global_config.h"
#define DELAY_BUFF_LEN 500

enum class sensor_e : uint8_t {
    P0,
    P1,

    F0,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,

    O2_IN,
    CO2_IN,
    O2_D1,
    CO2_D1,
    O2_D2,
    CO2_D2,
    O2_I1,
    CO2_I1,
    O2_I2,
    CO2_I2,
    O2_OUT,
    CO2_OUT,

    SENSOR_NUM,
};

typedef struct {
    uint16_t pressure;
    uint16_t gas[6];  // IN, D1,D2,I1,I2,OUT
} sensor_delays_t;

class SensorManager {
   public:
    static void init();
    static bool buffers_filled();
    // static float get_reading(sensor_e sensor);
    static float get_gas_reading(gas_sensor_e sensor);
    static float get_hp_reading(uint8_t i);
    static float get_flow_reading(uint8_t i);
    static float get_total_flow_reading();
    static float get_volume_reading(uint8_t i);
    static float get_total_volume_reading();
    static void set_delay_offset(uint16_t offset);
    static void process(void* obj);

   private:
    typedef struct {
        uint16_t del;     // Retraso del sensor en mseg
        uint32_t period;  // Periodo de lectura
        uint32_t last_read_t;
        SensorBuff<float> sensor_buff;
    } sensor_t;

    static sensor_t gas_sensors[GAS_SENSOR_NUM];
    static sensor_t flow_sensors[SENSOR_NUM_FLOW];
    static sensor_t hp_sensors[SENSOR_NUM_HP];
    static sensor_t total_flow;
    static sensor_t volumes[SENSOR_NUM_FLOW];
    static sensor_t total_volume;

    // static sensor_t sensors[(uint8_t)sensor_e::SENSOR_NUM];
    static bool _buffers_filled;
    static uint32_t gas_period;
    static uint32_t lp_period;
    static uint32_t temp_period;
    static uint32_t hp_period;
    static sensor_delays_t delays;

    // static float total_flow;
    static uint16_t delay_offset;

    static void init_sensor_struct(sensor_t& sensor, uint32_t period, uint16_t del, uint16_t max_del);
};

#endif