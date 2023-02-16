#include "HighPressureSensors.h"
#include "error.h"
#include "PinDef.h"
#include "utils.h"
#include "storage.h"

const float HighPressureSensors::c0 = 10.197162f;
const float HighPressureSensors::c1 = (3.3f / (8191.0f * 0.057f * 5.0f)) * c0;
const float HighPressureSensors::c2 = ((1.8f / 5.0f - 0.5f) / 0.057f) * c0;
float HighPressureSensors::c3[2] = {c2, c2};
const uint8_t HighPressureSensors::pins[2] = {PIN_P1, PIN_P2};
uint16_t HighPressureSensors::sample_n = 20;

void HighPressureSensors::load_zero() {
    set_zero(0, Storage::read_p_zero(0));
    set_zero(1, Storage::read_p_zero(1));
}

float HighPressureSensors::cal_zero(uint8_t sensor_i, bool save) {
    assert(sensor_i <= 1);
    c3[sensor_i] = c2;
    float zero = read_pressure(sensor_i, 500);
    set_zero(sensor_i, zero);
    if (save) {
        Storage::write_p_zero(sensor_i, zero);
    }
    return zero;
}

void HighPressureSensors::set_zero(uint8_t sensor_i, float zero) {
    assert(sensor_i <= 1);
    c3[sensor_i] = c2 - zero;
    printlnfd(Serial, "Pressure sensor #%u zero set: %.2f", sensor_i, zero);
}

float HighPressureSensors::read_pressure(uint8_t sensor_i) {
    assert(sensor_i <= 1);
    return read_pressure(sensor_i, sample_n);
}

float HighPressureSensors::read_pressure(uint8_t sensor_i, uint16_t n) {
    assert(sensor_i <= 1);
    const float r = read_raw_avg(sensor_i, n);
    const float p = r * c1 + c3[sensor_i];
    return p;
}

float HighPressureSensors::read_raw_avg(uint8_t sensor_i, uint16_t n) {
    assert(sensor_i <= 1);
    uint32_t sum = 0;
    for (uint16_t i = 0; i < n; i++) {
        sum += analogRead(pins[sensor_i]);
    }
    return (float)sum / (float)n;
}

void HighPressureSensors::set_sample_n(uint16_t n) {
    assert(n);
    sample_n = n;
}