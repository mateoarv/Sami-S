#include "storage.h"
#include <EEPROM.h>

#define ADDR_SENSOR_CAL 0
#define SENSOR_CAL_LEN sizeof(sensor_cal_t)
#define ADDR_P_ZERO (ADDR_SENSOR_CAL + SENSOR_CAL_LEN * 8)
#define P_ZERO_LEN sizeof(float)
#define ADDR_P_CAL (ADDR_P_ZERO + P_ZERO_LEN * 2)
#define P_CAL_LEN sizeof(gas_p_cal_t)

void Storage::write_flow_cal(uint8_t sensor_i, const sensor_cal_t& cal) {
    const uint32_t addr = ADDR_SENSOR_CAL + sensor_i * SENSOR_CAL_LEN;
    EEPROM.put(addr, cal);
}

void Storage::read_flow_cal(uint8_t sensor_i, sensor_cal_t& cal) {
    const uint32_t addr = ADDR_SENSOR_CAL + sensor_i * SENSOR_CAL_LEN;
    EEPROM.get(addr, cal);
}

void Storage::write_p_zero(uint8_t sensor_i, float zero) {
    const uint32_t addr = ADDR_P_ZERO + sensor_i * P_ZERO_LEN;
    EEPROM.put(addr, zero);
}

float Storage::read_p_zero(uint8_t sensor_i) {
    const uint32_t addr = ADDR_P_ZERO + sensor_i * P_ZERO_LEN;
    float zero = 0;
    EEPROM.get(addr, zero);
    return zero;
}

void Storage::write_gas_p_cal(uint8_t sensor_i, const gas_p_cal_t& cal) {
    const uint32_t addr = ADDR_P_CAL + sensor_i * P_CAL_LEN;
    EEPROM.put(addr, cal);
}
void Storage::read_gas_p_cal(uint8_t sensor_i, gas_p_cal_t& cal) {
    const uint32_t addr = ADDR_P_CAL + sensor_i * P_CAL_LEN;
    EEPROM.get(addr, cal);
}