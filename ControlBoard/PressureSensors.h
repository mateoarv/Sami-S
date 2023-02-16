#ifndef PRESSURE_SENSORS_H
#define PRESSURE_SENSORS_H
#include "Arduino.h"

class PressureSensors {
   public:
    static bool init();
    static float read_pressure(uint8_t n);
    static int16_t read_pressure_raw(uint8_t n);
    static float read_flow(uint8_t n);
    static float read_temp(uint8_t n);
    static void read_both(uint8_t n, float& pressure, float& temp);
    static void calibrate_zero();

   private:
    static bool select_sensor(uint8_t n);
    static bool get_product_number(uint32_t& dest);
    static void reset_sensor();

    static bool send_cmd(uint16_t cmd);
    static bool read_n(uint8_t n, uint16_t* dest);
    static int16_t zero_offsets[8];
};
#endif