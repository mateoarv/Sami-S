#include "GasSensors.h"
#include "utils.h"
#include "storage.h"
#include "HighPressureSensors.h"

GasSensors::sensor_t GasSensors::sensors[GAS_SENSOR_NUM];
uint16_t GasSensors::period = 10;
static const gas_p_cal_t default_p_cal = {
    {0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}};
const char* sensor_names[12] = {
    "O2_IN",
    "CO2_IN",
    "O2_D1",
    "CO2_D1",
    "O2_D2",
    "CO2_D2",
    "O2_I1",
    "CO2_I1",
    "O2_I2",
    "CO2_I2",
    "O2_OUT",
    "CO2_OUT"};

void GasSensors::init() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        sensors[i].p_cal = default_p_cal;
        sensors[i].p_cal_set = false;
    }
    query_nodes();
    load_p_cal();
}

void GasSensors::load_p_cal() {
    const uint8_t i = (uint8_t)gas_sensor_e::O2_IN;
    Storage::read_gas_p_cal(i, sensors[i].p_cal);
    sensors[i].p_cal_set = true;
    const gas_p_cal_t& cal = sensors[i].p_cal;
    printlnd("P cal read:");
    for (uint8_t i = 0; i < GAS_P_CAL_SAMPLES; i++) {
        printlnfd(Serial, "%.4f %.4f", cal.p[i], cal.o2_factor[i]);
    }
}

void GasSensors::reset_p_cal(gas_sensor_e sensor) {
    sensors[(uint8_t)sensor].p_cal_set = false;
}

void GasSensors::process(void* obj) {
    static uint32_t t0 = millis();
    static bool is_init = false;
    if (!is_init) {
        init();
        is_init = true;
        return;
    }
    if (millis() - t0 >= period) {
        t0 = millis();
        // read_all();
    }
}

// Se está demorando 264ms en leer todos los sensores
void GasSensors::read_all() {
    // Processes::enter_critical();
    // uint32_t t0 = Utils::tick();
    uint8_t failed = 0;
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        if (sensors[i].enabled) {
            float aux = 0;
            if (!read_sensor((gas_sensor_e)i, aux)) {
                printlnfd(Serial, "Gas sensor #%u read failed", i);
                failed++;
            } else {
                // printd(aux);
                // printd(", ");
            }
        }
    }
    // printlnd();
    //  printlnfd(Serial, "Failed: %u", failed);
    propagate_readings();
    // Utils::tock(t0, "Gas dt: ");
    // Processes::exit_critical();
}

void GasSensors::propagate_readings() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        if (sensors[i].propagate_pending) {
            sensors[i].reading = sensors[i].aux_reading;
            sensors[i].max_reading = max(sensors[i].max_reading, sensors[i].reading);
            sensors[i].min_reading = min(sensors[i].min_reading, sensors[i].reading);
            sensors[i].propagate_pending = false;
        }
    }
}

bool GasSensors::read_sensor(gas_sensor_e sensor, float& dest) {
    const uint8_t i = (uint8_t)sensor;
    /*if (millis() - sensors[i].last_reading_t < 15) {
        dest = sensors[i].reading;
        return true;
    }*/
    Comm::send(i, COMM_MSG_TYPE_READ_VAL, 1);
    comm_cmd_t cmd = {};

    if (wait_for_comm_resp(i, cmd)) {
        if (cmd.data_len != 3) return false;
        int16_t val = (int16_t)((cmd.data[1] << 8) + cmd.data[0]);
        sensors[i].flags = cmd.data[2];

        if (sensors[i].type == sensor_type_e::O2) {
            float p_factor = 1.0f;
            if (sensors[i].p_cal_set) {
                float p = HighPressureSensors::read_pressure(0);
                p_factor = interpolate(sensors[i].p_cal.p, sensors[i].p_cal.o2_factor, GAS_P_CAL_SAMPLES, p, false);
            }
            sensors[i].aux_reading = ((float)val) * 0.1f * p_factor;
        } else {
            sensors[i].aux_reading = ((float)val) * 0.01f;
        }
        sensors[i].propagate_pending = true;
        sensors[i].last_reading_t = millis();
        dest = sensors[i].aux_reading;
        return true;
    }
    return false;
}

void GasSensors::reset_min_max() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        reset_min_max((gas_sensor_e)i);
    }
}

void GasSensors::reset_min_max(gas_sensor_e sensor) {
    const float r = sensors[(uint8_t)sensor].reading;
    sensors[(uint8_t)sensor].max_reading = r;
    sensors[(uint8_t)sensor].min_reading = r;
}

float GasSensors::get_reading(gas_sensor_e sensor) {
    return sensors[(uint8_t)sensor].reading;
}

float GasSensors::get_max_reading(gas_sensor_e sensor) {
    return sensors[(uint8_t)sensor].max_reading;
}
float GasSensors::get_min_reading(gas_sensor_e sensor) {
    return sensors[(uint8_t)sensor].min_reading;
}

uint8_t GasSensors::get_flags(gas_sensor_e sensor) {
    return sensors[(uint8_t)sensor].flags;
}

sensor_type_e GasSensors::get_sensor_type(gas_sensor_e sensor) {
    return sensors[(uint8_t)sensor].type;
}

void GasSensors::query_nodes() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        sensor_type_e sensor_type;
        if (ping_node(i) && query_sensor_type(i, sensor_type)) {
            printd(sensor_type == sensor_type_e::O2 ? "O2" : "CO2");
            printlnfd(Serial, " sensor node with id #%u found", i);
            sensors[i].type = sensor_type;
            sensors[i].enabled = true;
        }
    }
}

bool GasSensors::ping_node(uint8_t node_id) {
    Comm::send(node_id, COMM_MSG_TYPE_PING);
    /*comm_cmd_t cmd = {};
    if (wait_for_comm_resp(node_id) && Comm::get_cmd(cmd)) {
        return cmd.data_len == 0;
    }
    return false;
    */
    return wait_for_empty_comm_resp(node_id);
}

bool GasSensors::query_sensor_type(uint8_t node_id, sensor_type_e& dest) {
    Comm::send(node_id, COMM_MSG_TYPE_READ_VAL, 0);
    comm_cmd_t cmd = {};
    if (wait_for_comm_resp(node_id, cmd)) {
        if (cmd.data_len != 1) return false;
        dest = (sensor_type_e)cmd.data[0];
        return true;
    }
    return false;
}

bool GasSensors::wait_for_comm_resp(uint8_t sender_id, comm_cmd_t& dest) {
    // static comm_cmd_t cmd = {};
    const uint32_t t0 = millis();
    while (!Comm::is_cmd_available()) {
        if (millis() - t0 >= COMM_RESP_TO) {
            printlnd("COMM RESP TIMEOUT");
            return false;
        }
        Processes::yield();
    }
    if (!Comm::get_cmd(dest)) return false;
    // if (!Comm::peek_cmd(cmd)) return false;
    if (dest.receiver_id != COMM_MASTER_ID ||
        dest.msg_type != COMM_MSG_TYPE_RESPONSE ||
        dest.sender_id != sender_id) {
        // Comm::get_cmd(cmd);
        //  printd(cmd.sender_id - sender_id);
        return false;
    }
    return true;
}
bool GasSensors::wait_for_empty_comm_resp(uint8_t sender_id) {
    comm_cmd_t cmd = {};
    if (wait_for_comm_resp(sender_id, cmd)) {
        return cmd.data_len == 0;
    }
    return false;
}

void GasSensors::O2_low_cal() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        O2_low_cal((gas_sensor_e)i);
    }
}
void GasSensors::O2_low_cal(gas_sensor_e sensor) {
    uint8_t i = (uint8_t)sensor;
    if (sensors[i].type == sensor_type_e::O2) {
        Comm::send(i, COMM_MSG_TYPE_O2_LOW_CAL);
        if (wait_for_empty_comm_resp(i)) {
            printlnfd(Serial, "O2 low cal started for sensor #%u", i);
        } else {
            printlnfd(Serial, "O2 low cal failed for sensor #%u", i);
        }
    }
}
void GasSensors::O2_high_cal() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        O2_high_cal((gas_sensor_e)i);
    }
}
void GasSensors::O2_high_cal(gas_sensor_e sensor) {
    uint8_t i = (uint8_t)sensor;
    if (sensors[i].type == sensor_type_e::O2) {
        Comm::send(i, COMM_MSG_TYPE_O2_HIGH_CAL);
        if (wait_for_empty_comm_resp(i)) {
            printlnfd(Serial, "O2 high cal started for sensor #%u", i);
        } else {
            printlnfd(Serial, "O2 high cal failed for sensor #%u", i);
        }
    }
}

void GasSensors::CO2_low_cal() {
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        CO2_low_cal((gas_sensor_e)i);
    }
}
void GasSensors::CO2_low_cal(gas_sensor_e sensor) {
    uint8_t i = (uint8_t)sensor;
    if (sensors[i].type == sensor_type_e::CO2) {
        Comm::send(i, COMM_MSG_TYPE_CO2_LOW_CAL);
        if (wait_for_empty_comm_resp(i)) {
            printlnfd(Serial, "CO2 low cal started for sensor #%u", i);
        } else {
            printlnfd(Serial, "CO2 low cal failed for sensor #%u", i);
        }
    }
}

bool GasSensors::is_warming() {
    // read_all();
    for (uint8_t i = 0; i < GAS_SENSOR_NUM; i++) {
        if (sensors[i].type == sensor_type_e::CO2 && (sensors[i].flags & 0b1000)) {
            return true;
        }
    }
    return false;
}