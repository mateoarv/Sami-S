#include "global_values.h"
#include "PressureSensors.h"

float GlobalValues::p_atm_bar = 0.84f;
bool GlobalValues::dbg_msgs_on = true;
void GlobalValues::set_p_atm_bar(float p_atm) {
    p_atm_bar = p_atm;
}

float GlobalValues::get_p_atm_bar() {
    return p_atm_bar;
}

float GlobalValues::get_temp_c() {
    static float temp = PressureSensors::read_temp(0);
    static uint32_t t0 = 0;

    if (millis() - t0 >= 60000) {
        temp = PressureSensors::read_temp(0);
        t0 = millis();
    }
    return temp;
    // return PressureSensors::read_temp(0);
}

bool GlobalValues::get_dbg_msgs_on() {
    return dbg_msgs_on;
}
void GlobalValues::set_dbg_msgs_on(bool on) {
    dbg_msgs_on = on;
}