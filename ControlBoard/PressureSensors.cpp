#include "PressureSensors.h"
#include "PinDef.h"
#include "Wire.h"
#include "utils.h"
#define SW_ADDR 0x70
#define SEN_ADDR 0x25
int16_t PressureSensors::zero_offsets[8];

bool PressureSensors::init() {
    // Resetear I2C switch
    digitalWrite(PIN_I2C_RST, LOW);
    delay(1);
    digitalWrite(PIN_I2C_RST, HIGH);
    delay(1);

    for (uint8_t i = 0; i < 8; i++) {
        select_sensor(i);
        send_cmd(0x3FF9);  // Stop sensor
        reset_sensor();
        uint32_t id = 0;
        if (!get_product_number(id) || id != 0x3020A01) {
            printlnd("test");
            return false;
        }
        /* Continuous modes:
            -0x3603: Mass flow, avg
            -0x3608: Mass flow
            -0x3615: Differential pressure, avg
            -0x361E: Differential pressure
        */
        if (!send_cmd(0x3615)) {
            return false;
        }
    }
    // delay(20);
    return true;
}

bool PressureSensors::select_sensor(uint8_t n) {
    Wire1.beginTransmission(SW_ADDR);
    if (n == 4) {
        n = 5;
    } else if (n == 5) {
        n = 4;
    } else if (n == 6) {
        n = 7;
    } else if (n == 7) {
        n = 6;
    }

    Wire1.write(0b1 << n);
    return !Wire1.endTransmission();
}

void PressureSensors::reset_sensor() {
    send_cmd(0x0006);
    delay(5);
}

bool PressureSensors::get_product_number(uint32_t& dest) {
    send_cmd(0x367C);
    send_cmd(0xE102);
    uint16_t data[6];
    if (!read_n(6, data)) {
        return false;
    }
    dest = (data[0] << 16) | (data[1] & 0xFFFF);
    return true;
}

float PressureSensors::read_temp(uint8_t n) {
    float pressure = 0;
    float temp = 0;
    read_both(n, pressure, temp);
    return temp;
}

int16_t PressureSensors::read_pressure_raw(uint8_t n) {
    select_sensor(n);
    int16_t data;
    if (read_n(1, (uint16_t*)&data)) {
        return data;
    }
    return 0;
}

float PressureSensors::read_pressure(uint8_t n) {
    return ((float)(read_pressure_raw(n) - zero_offsets[n])) / 60.0f;
    /*select_sensor(n);
    int16_t data;
    if (read_n(1, (uint16_t*)&data)) {
        return data / 60.0;  // Pa
    }
    return 0;
    */
}

void PressureSensors::read_both(uint8_t n, float& pressure, float& temp) {
    select_sensor(n);
    int16_t data[2];
    if (read_n(3, (uint16_t*)data)) {
        pressure = ((float)(data[0] - zero_offsets[n])) / 60.0f;  // Pa
        temp = data[1] / 200.0;                                   // C
        // printlnd(data[2]);
    }
}

float PressureSensors::read_flow(uint8_t n) {
    return ((float)read_pressure_raw(n)) / 9000.0f;
    /*select_sensor(n);
    int16_t data;
    if (read_n(1, (uint16_t*)&data)) {
        return (data / 9000.0);  // l/s
    }
    return 0;*/
}

void PressureSensors::calibrate_zero() {
    const uint8_t samples = 100;
    for (uint8_t i = 0; i < 8; i++) {
        int16_t avg = 0;
        for (uint8_t j = 0; j < samples; j++) {
            avg += read_pressure_raw(i);
            delay(1);
        }
        avg /= samples;
        zero_offsets[i] = avg;
        printlnfd(Serial, "Flow sensor %u zero offset: %i", i, avg);
    }
}

bool PressureSensors::send_cmd(uint16_t cmd) {
    Wire1.beginTransmission(SEN_ADDR);
    Wire1.write(highByte(cmd));
    Wire1.write(lowByte(cmd));
    return !Wire1.endTransmission();
}

bool PressureSensors::read_n(uint8_t n, uint16_t* dest) {
    uint8_t nb = n * 3;
    Wire1.requestFrom((uint8_t)SEN_ADDR, nb);
    if (Wire1.available() == nb) {
        for (uint8_t i = 0; i < n; i++) {
            uint8_t msb = Wire1.read();
            uint8_t lsb = Wire1.read();
            Wire1.read();
            dest[i] = (msb << 8) | (lsb & 0xFF);
        }
        return true;
    }
    return false;
}