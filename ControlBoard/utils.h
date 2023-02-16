#ifndef UTILS_H
#define UTILS_H

#include "Arduino.h"
#include "flow_sensors.h"
#define IS_NUM(c) (c >= 48 && c <= 57)

void print(const char* msg);
void print(char c);
void println(const char* msg);
void println();
void printd(const char* msg);
void printd(char c);
void printlnd(const char* msg);
void printlnd();
int printf(Print& serial, const char* format, ...);
int printlnf(Print& serial, const char* format, ...);
int printfd(Print& serial, const char* format, ...);
int printlnfd(Print& serial, const char* format, ...);
void to_upper(char* str);
float interpolate(const float* in_vals, const float* out_vals, uint8_t len, float val, bool mirror);
void print_sensor_cal(const sensor_cal_t& cal);
float ATPD_to_STPD0(float p_atm_bar, float temp_c, float p_atpd);
float ATPD_to_STPD0(float p_atpd);
float STPD0_to_ATPD(float p_atm_bar, float temp_c, float p_stpd0);
float STPD0_to_ATPD(float p_stpd0);

class Utils {
   public:
    static uint32_t tick();
    static uint32_t tock(uint32_t t0, bool print);
    static uint32_t tock(uint32_t t0, const char* msg);
};

#endif