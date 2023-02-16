#include "SerialCmds.h"
#include "GasSensors.h"
#include "program.h"
#include "PressureSensors.h"
#include "flow_sensors.h"
#include "UI.h"
#include "HighPressureSensors.h"
#include "Ventilation.h"
#include "global_values.h"
#include "sensor_manager.h"
char SerialCmds::in_buff[SERIAL_BUFF_LEN];
uint8_t SerialCmds::buff_i = 0;
SerialCmds::field_t SerialCmds::fields[MAX_CMD_FIELDS];
uint8_t SerialCmds::n_fields;
const char* SerialCmds::int_pattern = "^[+-]?\\d+$";
const char* SerialCmds::float_pattern = "^[+-]?\\d*\\.?\\d+$";
bool SerialCmds::cmd_ready = false;

/*
void SerialCmds::init() {
    // int_pattern = re_compile("^[+-]?\\d+$");
    //  float_pattern = re_compile("^[+-]?\\d*\\.?\\d+$");

    // re_t test_pat = re_compile("^[+-]?\d*\.?\d$");
    int match_len = 0;
    int pos = 0;
    pos = re_match(float_pattern, "12.23", &match_len);
    printlnf(Serial, "Pos: %i Len: %u", pos, match_len);
    pos = re_match(float_pattern, "10", &match_len);
    printlnf(Serial, "Pos: %i Len: %u", pos, match_len);
    pos = re_match(float_pattern, "05", &match_len);
    printlnf(Serial, "Pos: %i Len: %u", pos, match_len);
    pos = re_match(float_pattern, "0", &match_len);
    printlnf(Serial, "Pos: %i Len: %u", pos, match_len);
    pos = re_match(float_pattern, "0.444", &match_len);
    printlnf(Serial, "Pos: %i Len: %u", pos, match_len);
}*/

void SerialCmds::process(void* obj) {
    while (Serial.available()) {
        char c = Serial.read();
        if (c != '\n' && c != '\r') {
            in_buff[buff_i] = c;
            if (++buff_i >= SERIAL_BUFF_LEN) {
                println("CMD OVERFLOW");
                buff_i = 0;
            }
        } else {
            if (c == '\n') {
                in_buff[buff_i] = '\0';
                buff_i = 0;
                extract_fields();
            }
        }
    }
}

void SerialCmds::extract_fields() {
    const uint8_t cmd_len = strlen(in_buff);
    int8_t field_i = -1;
    n_fields = 0;
    cmd_ready = false;
    bool cmd_ok = true;
    for (uint8_t i = 0; i < cmd_len; i++) {
        if (in_buff[i] != ' ' && field_i == -1) {
            field_i = i;
        }
        if ((in_buff[i] == ' ') || ((i + 1) == cmd_len)) {
            if (field_i != -1) {
                if (n_fields >= MAX_CMD_FIELDS) {
                    println("CMD FIELD OVERFLOW");
                    cmd_ok = false;
                    break;
                }
                fields[n_fields].i = field_i;
                fields[n_fields].j = (in_buff[i] == ' ') ? i : i + 1;
                assign_field_type(fields[n_fields]);
                field_i = -1;
                n_fields++;
            } else {
                println("INVALID CMD SYNTAX");
                cmd_ok = false;
                break;
            }
        }
    }
    if (cmd_ok) {
        to_upper(in_buff);
        check_cmd();
    }
}

void SerialCmds::check_cmd() {
    // print_cmd();
    if (n_fields == 0 || fields[0].type != STR) {
        goto INVALID;
    }
    if (strncmp(in_buff, "AIR", fields[0].j) == 0) {
        float rate = 0;
        if (n_fields != 2 || fields[1].type == STR) {
            goto INVALID;
        }
        get_field(rate, 1);
        Program::set_air_f(rate);
    } else if (strncmp(in_buff, "N2", fields[0].j) == 0) {
        float rate = 0;
        if (n_fields != 2 || fields[1].type == STR) {
            goto INVALID;
        }
        get_field(rate, 1);
        Program::set_N2_f(rate);
    } else if (strncmp(in_buff, "CO2", fields[0].j) == 0) {
        float rate = 0;
        if (n_fields != 2 || fields[1].type == STR) {
            goto INVALID;
        }
        get_field(rate, 1);
        Program::set_CO2_f(rate);
    } else if (strncmp(in_buff, "SENF", fields[0].j) == 0) {
        if (n_fields == 1) {
            Program::set_sen_f(0.18f);
        } else if (n_fields == 2) {
            float rate = 0;
            get_field(rate, 1);
            Program::set_sen_f(rate);
        } else {
            goto INVALID;
        }

    } else if (strncmp(in_buff, "PUMP", fields[0].j) == 0) {
        int32_t perc = 0;
        if (n_fields != 2 || fields[1].type != INT) {
            goto INVALID;
        }
        get_field(perc, 1);
        if (perc < 0 || perc > 100) {
            goto INVALID;
        }
        Program::set_pump(perc);
    } else if (strncmp(in_buff, "ATM", fields[0].j) == 0) {
        float p = 0;
        if (n_fields != 2 || fields[1].type == STR) {
            goto INVALID;
        }
        get_field(p, 1);
        if (p < 0) {
            goto INVALID;
        }
        GlobalValues::set_p_atm_bar(p);
        // GasSensors::set_atm_p(p);
    } else if (strncmp(in_buff, "CLOSE", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        Program::close_gases();
    } else if (strncmp(in_buff, "STOP", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        Program::stop_all();
    } else if (strncmp(in_buff, "FLOW_ZERO", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        PressureSensors::calibrate_zero();
    } else if (strncmp(in_buff, "FTDI", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        Program::ftdi_mode();
    } else if (strncmp(in_buff, "PL_O2_IN", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(0, state);
    } else if (strncmp(in_buff, "PL_CO2_IN", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(1, state);
    } else if (strncmp(in_buff, "PL_O2_D1", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(2, state);
    } else if (strncmp(in_buff, "PL_CO2_D1", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(3, state);
    } else if (strncmp(in_buff, "PL_O2_D2", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(4, state);
    } else if (strncmp(in_buff, "PL_CO2_D2", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(5, state);
    } else if (strncmp(in_buff, "PL_O2_I1", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(6, state);
    } else if (strncmp(in_buff, "PL_CO2_I1", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(7, state);
    } else if (strncmp(in_buff, "PL_O2_I2", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(8, state);
    } else if (strncmp(in_buff, "PL_CO2_I2", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(9, state);
    } else if (strncmp(in_buff, "PL_O2_OUT", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(10, state);
    } else if (strncmp(in_buff, "PL_CO2_OUT", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot(11, state);
    } else if (strncmp(in_buff, "PL_CTRL", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_plot_control(state);
    } else if (strncmp(in_buff, "PL_FLOW", fields[0].j) == 0) {
        if (n_fields != 3) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        bool state = false;
        get_field(sensor_i, 1);
        get_field(state, 2);
        Program::set_flow_plot(sensor_i, state);
    } else if (strncmp(in_buff, "PL_VOL", fields[0].j) == 0) {
        if (n_fields != 3) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        bool state = false;
        get_field(sensor_i, 1);
        get_field(state, 2);
        Program::set_volume_plot(sensor_i, state);
    } else if (strncmp(in_buff, "PL_TOTAL_VOL", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_volume_plot(8, state);
    } else if (strncmp(in_buff, "PL_TOTAL_FLOW", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_flow_plot(8, state);
    } else if (strncmp(in_buff, "PL_P", fields[0].j) == 0) {
        if (n_fields != 3) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        bool state = false;
        get_field(sensor_i, 1);
        get_field(state, 2);
        Program::set_pressure_plot(sensor_i, state);
    } else if (strncmp(in_buff, "PL_DAB_O2", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_DAB_O2_plot(state);
    } else if (strncmp(in_buff, "PL_DAB_CO2", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_DAB_CO2_plot(state);
    } else if (strncmp(in_buff, "PL_CLEAR", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        for (uint8_t i = 0; i < 12; i++) {
            Program::set_plot(i, false);
            if (i < 2) {
                Program::set_pressure_plot(i, false);
            }
            if (i < 8) {
                Program::set_flow_plot(i, false);
                Program::set_volume_plot(i, false);
            }
        }
        Program::set_flow_plot(8, false);
        Program::set_volume_plot(8, false);
        Program::set_DAB_O2_plot(false);
        Program::set_DAB_CO2_plot(false);
    } else if (strncmp(in_buff, "PL_ALL", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        for (uint8_t i = 0; i < 12; i++) {
            Program::set_plot(i, true);
            if (i < 2) {
                Program::set_pressure_plot(i, true);
            }
            if (i < 4) {
                Program::set_flow_plot(i, true);
                Program::set_volume_plot(i, true);
            }
        }
        Program::set_flow_plot(8, true);
        Program::set_volume_plot(8, true);
        Program::set_DAB_O2_plot(true);
        Program::set_DAB_CO2_plot(true);
    } else if (strncmp(in_buff, "O2_LOW_CAL", fields[0].j) == 0) {
        if (n_fields == 1) {
            GasSensors::O2_low_cal();
        } else if (n_fields == 2) {
            uint32_t n = 0;
            get_field(n, 1);
            GasSensors::O2_low_cal((gas_sensor_e)n);
        } else {
            goto INVALID;
        }
    } else if (strncmp(in_buff, "O2_HIGH_CAL", fields[0].j) == 0) {
        if (n_fields == 1) {
            GasSensors::O2_high_cal();
        } else if (n_fields == 2) {
            uint32_t n = 0;
            get_field(n, 1);
            GasSensors::O2_high_cal((gas_sensor_e)n);
        } else {
            goto INVALID;
        }
    } else if (strncmp(in_buff, "CO2_LOW_CAL", fields[0].j) == 0) {
        if (n_fields == 1) {
            GasSensors::CO2_low_cal();
        } else if (n_fields == 2) {
            uint32_t n = 0;
            get_field(n, 1);
            GasSensors::CO2_low_cal((gas_sensor_e)n);
        } else {
            goto INVALID;
        }
    } else if (strncmp(in_buff, "SP_O2", fields[0].j) == 0) {
        float rate = 0;  // ml/min
        if (n_fields != 2 || fields[1].type == STR) {
            goto INVALID;
        }
        get_field(rate, 1);
        Program::set_sp_o2(rate / 1000.0f);

    } else if (strncmp(in_buff, "CONTROL", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        bool state = false;
        get_field(state, 1);
        Program::set_controller(state);
    } else if (strncmp(in_buff, "WARMING?", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        printlnf(Serial, "Warming: %u", GasSensors::is_warming());
    } else if (strncmp(in_buff, "WAIT_WARM", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        while (GasSensors::is_warming()) {
            printlnd("Waiting for warmup...");
            delay(5000);
        }
        printlnd("Warmup complete");
    } else if (strncmp(in_buff, "FLOW_AVG", fields[0].j) == 0) {
        if (n_fields != 3) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        uint32_t time = 0;
        get_field(sensor_i, 1);
        get_field(time, 2);
        printlnf(Serial, "Flow raw avg: %.2f", FlowSensors::get_raw_avg(sensor_i, time));
        // UI::buzz(1, 100);
    } else if (strncmp(in_buff, "GET_FLOW", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        get_field(sensor_i, 1);
        printlnf(Serial, "Flow: %.2f", FlowSensors::get_flow(sensor_i));
    } else if (strncmp(in_buff, "START_FLOW_CAL", fields[0].j) == 0) {
        if (n_fields != 3) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        uint32_t time = 0;
        get_field(sensor_i, 1);
        get_field(time, 2);
        FlowSensors::start_cal_rutine(sensor_i, time);
    } else if (strncmp(in_buff, "CAL_MEASURE", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        FlowSensors::start_cal_measurement();
    } else if (strncmp(in_buff, "CAL_VAL", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        float out_val = 0;
        get_field(out_val, 1);
        FlowSensors::set_measurement_out(out_val);
    } else if (strncmp(in_buff, "RESTORE_DEF_CAL", fields[0].j) == 0) {
        if (n_fields == 1) {
            FlowSensors::restore_def_cal();
        } else if (n_fields == 2) {
            uint8_t sensor_i = 0;
            get_field(sensor_i, 1);
            FlowSensors::restore_def_cal(sensor_i);
        } else {
            goto INVALID;
        }
    } else if (strncmp(in_buff, "GET_CAL", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        uint8_t sensor_i = 0;
        get_field(sensor_i, 1);
        FlowSensors::print_sensor_cal(sensor_i);
    } else if (strncmp(in_buff, "SAVE_CAL", fields[0].j) == 0) {
        if (n_fields == 1) {
            FlowSensors::save_sensor_cal();
        } else if (n_fields == 2) {
            uint8_t sensor_i = 0;
            get_field(sensor_i, 1);
            FlowSensors::save_sensor_cal(sensor_i);
        } else {
            goto INVALID;
        }
    } else if (strncmp(in_buff, "PRESSURE_ZERO", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        HighPressureSensors::cal_zero(0, true);
        HighPressureSensors::cal_zero(1, true);
    } else if (strncmp(in_buff, "SET_P_SAMPLES", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        uint32_t n = 0;
        get_field(n, 1);
        HighPressureSensors::set_sample_n(n);
    } else if (strncmp(in_buff, "GET_DAB_O2", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        printlnf(Serial, "Delta AB O2: %.1f", Ventilation::get_delta_ab_O2());
    } else if (strncmp(in_buff, "GET_DAB_CO2", fields[0].j) == 0) {
        if (n_fields != 1) {
            goto INVALID;
        }
        printlnf(Serial, "Delta AB CO2: %.1f", Ventilation::get_delta_ab_CO2());
    } else if (strncmp(in_buff, "GET_TEMP", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        uint8_t i = 0;
        get_field(i, 1);
        printlnf(Serial, "Temperature: %.2fC", PressureSensors::read_temp(i));
    } else if (strncmp(in_buff, "SET_DEL_OFF", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        uint32_t offset = 0;
        get_field(offset, 1);
        SensorManager::set_delay_offset(offset);
    } else if (strncmp(in_buff, "SET_DBG", fields[0].j) == 0) {
        if (n_fields != 2) {
            goto INVALID;
        }
        uint8_t enabled = 0;
        get_field(enabled, 1);
        GlobalValues::set_dbg_msgs_on(enabled);
    } else {
        goto INVALID;
    }
    cmd_ready = true;
    return;

INVALID:
    println("INVALID CMD");
    print_cmd();
}

void SerialCmds::assign_field_type(field_t& field) {
    char aux[MAX_FIELD_LEN + 1];
    strncpy(aux, in_buff + field.i, field.j - field.i);
    aux[field.j - field.i] = '\0';
    int match_len = 0;

    if (re_match(int_pattern, aux, &match_len) != -1) {
        field.type = INT;
    } else if (re_match(float_pattern, aux, &match_len) != -1) {
        field.type = FLOAT;
    } else {
        field.type = STR;
    }
}

bool SerialCmds::get_field(char* dest, uint8_t i) {
    if (n_fields <= i) {
        return false;
    }

    const uint8_t field_len = fields[i].j - fields[i].i;
    strncpy(dest, in_buff + fields[i].i, field_len);
    dest[field_len] = '\0';
    return true;
}

bool SerialCmds::get_field(float& dest, uint8_t i) {
    char aux[MAX_FIELD_LEN];
    if (!get_field(aux, i)) {
        return false;
    }
    dest = atof(aux);
    return true;
}

bool SerialCmds::get_field(uint8_t& dest, uint8_t i) {
    char aux[MAX_FIELD_LEN];
    if (!get_field(aux, i)) {
        return false;
    }
    dest = atoi(aux);
    return true;
}

bool SerialCmds::get_field(uint32_t& dest, uint8_t i) {
    char aux[MAX_FIELD_LEN];
    if (!get_field(aux, i)) {
        return false;
    }
    dest = atoi(aux);
    return true;
}

bool SerialCmds::get_field(int32_t& dest, uint8_t i) {
    char aux[MAX_FIELD_LEN];
    if (!get_field(aux, i)) {
        return false;
    }
    dest = atoi(aux);
    return true;
}

bool SerialCmds::get_field(bool& dest, uint8_t i) {
    char aux[MAX_FIELD_LEN];
    if (!get_field(aux, i)) {
        return false;
    }
    dest = (atoi(aux) == 1);
    return true;
}

bool SerialCmds::cmd_available() {
    return cmd_ready;
}

void SerialCmds::print_cmd() {
    printlnf(Serial, "Fields: %u", n_fields);
    for (uint8_t i = 0; i < n_fields; i++) {
        printfd(Serial, "%u. Content: ", i);
        for (uint8_t z = fields[i].i; z < fields[i].j; z++) {
            print(in_buff[z]);
        }
        print(" Type: ");
        if (fields[i].type == STR) {
            println("STR");
        } else if (fields[i].type == INT) {
            println("INT");
        } else if (fields[i].type == FLOAT) {
            println("FLOAT");
        }
    }
}