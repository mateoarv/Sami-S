#include "utils.h"
#include "printf.h"
#include "global_values.h"
static void put_c(char c, void* extra_arg);

void print(const char* msg) {
    Serial.print(msg);
}
void print(char c) {
    Serial.print(c);
}
void println(const char* msg) {
    Serial.println(msg);
}
void println() {
    Serial.println();
}
void printd(const char* msg) {
    if (GlobalValues::get_dbg_msgs_on()) {
        Serial.print(msg);
    }
}
void printd(char c) {
    if (GlobalValues::get_dbg_msgs_on()) {
        Serial.print(c);
    }
}
void printlnd(const char* msg) {
    if (GlobalValues::get_dbg_msgs_on()) {
        Serial.println(msg);
    }
}
void printlnd() {
    if (GlobalValues::get_dbg_msgs_on()) {
        Serial.println();
    }
}

int printf(Print& serial, const char* format, ...) {
    va_list va;
    va_start(va, format);
    const int ret = vfctprintf(put_c, &serial, format, va);
    return ret;
}

int printlnf(Print& serial, const char* format, ...) {
    va_list va;
    va_start(va, format);
    const int ret = vfctprintf(put_c, &serial, format, va);
    serial.println();
    return ret;
}

int printfd(Print& serial, const char* format, ...) {
    if (GlobalValues::get_dbg_msgs_on()) {
        va_list va;
        va_start(va, format);
        const int ret = vfctprintf(put_c, &serial, format, va);
        return ret;
    }
    return 0;
}

int printlnfd(Print& serial, const char* format, ...) {
    if (GlobalValues::get_dbg_msgs_on()) {
        va_list va;
        va_start(va, format);
        const int ret = vfctprintf(put_c, &serial, format, va);
        serial.println();
        return ret;
    }
    return 0;
}

void to_upper(char* str) {
    const uint16_t len = strlen(str);
    for (uint8_t i = 0; i < len; i++) {
        char c = str[i];
        if (c >= 97 && c <= 122) {
            str[i] = (c - 32);
        }
    }
}

static void put_c(char c, void* extra_arg) {
    ((Print*)extra_arg)->print(c);
}

float interpolate(const float* in_vals, const float* out_vals, uint8_t len, float val, bool mirror) {
    bool neg = false;
    if (mirror && val < 0) {
        neg = true;
        val *= (-1);
    }

    // Encontrar index
    uint8_t index = 1;
    while (in_vals[index] < val && index < len - 1)
        index++;

    // Interpolar valor
    const float x0 = in_vals[index - 1];
    const float y0 = out_vals[index - 1];
    const float x1 = in_vals[index];
    const float y1 = out_vals[index];
    float res = y0 + (val - x0) * ((y1 - y0) / (x1 - x0));
    return neg ? -res : res;
}

void print_sensor_cal(const sensor_cal_t& cal) {
    Serial.println("Sensor calibration values: (Out In)");
    for (uint8_t i = 0; i < FLOW_CALIB_SAMPLES; i++) {
        printlnfd(Serial, "%.2f  %.2f", cal.out_vals[i], cal.in_vals[i]);
    }
    printlnfd(Serial, "Zero Offset: %.2f", cal.zero_offset);
}

uint32_t Utils::tick() {
    return millis();
}

uint32_t Utils::tock(uint32_t t0, bool print) {
    uint32_t dt = millis() - t0;
    if (print) {
        printlnfd(Serial, "Dt: %u", dt);
    }
    return dt;
}

uint32_t Utils::tock(uint32_t t0, const char* msg) {
    uint32_t dt = tock(t0, false);
    printlnfd(Serial, "%s%u", msg, dt);
    return dt;
}

float ATPD_to_STPD0(float p_atm_bar, float temp_c, float p_atpd) {
    return (273.15f / (273.15f + temp_c)) * (p_atm_bar / 1.01308f) * p_atpd;
}

float ATPD_to_STPD0(float p_atpd) {
    return (273.15f / (273.15f + GlobalValues::get_temp_c())) * (GlobalValues::get_p_atm_bar() / 1.01308f) * p_atpd;
}

float STPD0_to_ATPD(float p_atm_bar, float temp_c, float p_stpd0) {
    return ((273.15f + temp_c) / 273.15f) * (1.01308f / p_atm_bar) * p_stpd0;
}

float STPD0_to_ATPD(float p_stpd0) {
    return ((273.15f + GlobalValues::get_temp_c()) / 273.15f) * (1.01308f / GlobalValues::get_p_atm_bar()) * p_stpd0;
}