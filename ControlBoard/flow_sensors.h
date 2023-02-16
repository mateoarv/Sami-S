#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H
#include "Arduino.h"

#define FLOW_CALIB_SAMPLES 19
#define FLOW_N_SENSORS 8

typedef struct {
    float in_vals[FLOW_CALIB_SAMPLES];
    float out_vals[FLOW_CALIB_SAMPLES];
    float zero_offset;
} sensor_cal_t;

class FlowSensors {
   public:
    static void init();
    static void restore_def_cal();
    static void restore_def_cal(uint8_t sensor_i);
    static void save_sensor_cal();
    static void save_sensor_cal(uint8_t sensor_i);
    static void print_sensor_cal(uint8_t sensor_i);
    static void set_sensor_enabled(uint8_t sensor_i, bool enabled);
    static void set_global_enabled(bool enabled);
    static float get_flow(uint8_t sensor_i);

    static float read_flow(uint8_t sensor_i);

    static float get_volume(uint8_t sensor_i);
    static float get_total_flow();
    static float get_total_volume();
    static bool get_ins();
    static void set_sample_period(uint16_t ms);
    static float get_raw_avg(uint8_t sensor_i, uint32_t time);
    static void start_cal_rutine(uint8_t sensor_i, uint32_t time);
    static void start_cal_measurement();
    static void set_measurement_out(float out_val);

   private:
    typedef struct {
        sensor_cal_t cal;
        bool enabled;
        float last_val;
        uint32_t last_t;
        float volume;
        float comp_volume;
        float max_vol_ins;
        float min_vol_esp;
        float ins_volume_mult;
        float current_volume_diff;
        float volume_diff;
        bool ins;
    } flow_sensor_t;

    static flow_sensor_t sensors[FLOW_N_SENSORS];
    // El sensor hace un promedio durante 25ms, es bueno que sample period
    // no sea mayor a este valor.
    static uint16_t sample_period;
    static sensor_cal_t def_cal;
    static bool global_enabled;
    static float total_flow;
    static float total_volume;
    static float total_comp_volume;
    static bool ins;

    static float cal_suggested_vals[FLOW_CALIB_SAMPLES];
    static bool cal_rutine_running;
    static uint8_t cal_rutine_sample_i;
    static uint8_t cal_rutine_sensor_i;
    static uint32_t cal_rutine_time;
    static float cal_measurement;
    static bool cal_measurement_ready;

    static void print_suggested_val(uint8_t i);
};

#endif