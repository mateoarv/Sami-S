#include "flow_controller.h"
#include "utils.h"
#define RESP_TO 1000

bool FlowController::init() {
    printlnd();
    if (cmd_id()) {
        printlnfd(Serial, "Controller #%u found, ID: %s", cont_num, id);
        has_id = true;
    } else {
        printlnfd(Serial, "Controller #%u not found.", cont_num);
        return false;
    }
    for (uint8_t i = 0; i < 4; i++) {
        if (!((cmd_write_measure_unit(i + 1, measure_unit_e::SL) && cmd_write_time_unit(i + 1, time_unit_e::MIN)))) {
            printlnfd(Serial, "Controller #%u channel #%u units failed", cont_num, i + 1);
            return false;
        }
        write_SP_rate_STPD0(i + 1, 0);
    }
    printlnfd(Serial, "Controller #%u units set.", cont_num);
    is_init = true;

    if (is_init) {
        /*
        float rate = 0.0f;
        if (read_SP_rate(1, rate)) {
            printlnfd(Serial, "Read rate: %.2f", rate);
        }
        write_SP_rate(1, 6.9f);
        */

        /*time_unit_e unit;
        cmd_read_time_unit(1, unit);
        if (unit == time_unit_e::MIN) {
            cmd_write_time_unit(1, time_unit_e::SEC);
        } else if (unit == time_unit_e::SEC) {
            cmd_write_time_unit(1, time_unit_e::MIN);
        }*/
    }
    return is_init;
}

void FlowController::static_process(void* obj) {
    assert(obj);
    ((FlowController*)obj)->process();
}

void FlowController::process() {
    while (serial.available()) {
        if (feed_c(serial.read())) {
            break;
        }
    }
}

bool FlowController::cmd_id() {
    serial.println("AZI");
    return wait_response(9) && get_field(1, id, ID_BUFF_LEN);
}

bool FlowController::cmd_read_value(uint8_t channel, bool input_port, uint8_t param) {
    assert(channel && channel <= 4);
    assert(has_id);
    const uint8_t port = input_port ? channel * 2 - 1 : channel * 2;
    printlnf(serial, "AZ%s.0%up%02u?", id, port, param);
    return wait_response(6) && get_field(4);
}

bool FlowController::cmd_read_value(uint8_t channel, bool input_port, uint8_t param, uint32_t& dest) {
    if (cmd_read_value(channel, input_port, param)) {
        dest = atoi(field_buff);
        return true;
    }
    return false;
}

bool FlowController::cmd_read_value(uint8_t channel, bool input_port, uint8_t param, float& dest) {
    if (cmd_read_value(channel, input_port, param)) {
        dest = atof(field_buff);
        return true;
    }
    return false;
}

bool FlowController::cmd_write_value(uint8_t channel, bool input_port, uint8_t param, const char* value) {
    assert(channel && channel <= 4);
    assert(has_id);
    const uint8_t port = input_port ? channel * 2 - 1 : channel * 2;
    printlnf(serial, "AZ%s.0%up%02u=%s", id, port, param, value);
    return wait_response(6);
}

bool FlowController::cmd_write_value(uint8_t channel, bool input_port, uint8_t param, uint32_t value) {
    snprintf_(aux_buff, AUX_BUFF_LEN, "%I32u", value);
    return cmd_write_value(channel, input_port, param, aux_buff);
}

bool FlowController::cmd_write_value(uint8_t channel, bool input_port, uint8_t param, float value) {
    snprintf_(aux_buff, AUX_BUFF_LEN, "%.2f", value);
    return cmd_write_value(channel, input_port, param, aux_buff);
}

bool FlowController::cmd_read_measure_unit(uint8_t channel, measure_unit_e& dest) {
    return cmd_read_value(channel, true, 4, (uint32_t&)dest);
}

bool FlowController::cmd_write_measure_unit(uint8_t channel, measure_unit_e unit) {
    return cmd_write_value(channel, true, 4, (uint32_t)unit);
}

bool FlowController::cmd_read_time_unit(uint8_t channel, time_unit_e& dest) {
    return cmd_read_value(channel, true, 10, (uint32_t&)dest);
}

bool FlowController::cmd_write_time_unit(uint8_t channel, time_unit_e unit) {
    return cmd_write_value(channel, true, 10, (uint32_t)unit);
}

bool FlowController::read_SP_rate(uint8_t channel, float& rate) {
    return cmd_read_value(channel, false, 1, rate);
}

bool FlowController::write_SP_rate_ATPD(uint8_t channel, float rate_ATPD) {
    return cmd_write_value(channel, false, 1, ATPD_to_STPD0(rate_ATPD));
}

bool FlowController::write_SP_rate_STPD0(uint8_t channel, float rate_STPD0) {
    return cmd_write_value(channel, false, 1, rate_STPD0);
}

bool FlowController::feed_c(char c) {
    static bool msg_complete = false;
    if (c != '\r' && c != '\n') {
        if (msg_pending) {
            printlnfd(Serial, "CONTROLLER #%u MSG MISSED", cont_num);
            msg_pending = false;
        }
        if (msg_complete) {
            msg_complete = false;
            msg_fields = 0;
            in_buff_i = 0;
        }
        if (c == ',' || msg_fields == 0) {
            msg_fields++;
        }
        in_buff[in_buff_i] = c;
        if (++in_buff_i >= IN_BUFF_LEN) {
            in_buff_i = 0;
            msg_fields = 0;
            printlnfd(Serial, "CONTROLLER #%u IN_BUFF OVERFLOW", cont_num);
        }
    } else {
        if (c == '\r') {
            // Validar msg
            if (in_buff_i >= 2 && (strncmp(in_buff, "AZ", 2) == 0)) {
                in_buff[in_buff_i] = '\0';
                msg_pending = true;
                msg_complete = true;
                // printlnfd(Serial, "CTRL #%u IN MSG: %s", cont_num, in_buff);
                return true;
            }
        }
    }
    return false;
}

bool FlowController::wait_response(uint8_t n_fields) {
    uint32_t t0 = millis();
    while (!msg_pending) {
        if (millis() - t0 >= RESP_TO) {
            printlnfd(Serial, "CONTROLLER #%u RESPONSE TIMEOUT", cont_num);
            return false;
        }
        Processes::process_all();
    }
    msg_pending = false;
    // Verificar ID
    if (is_init) {
        if (!get_field(1) || (strncmp(field_buff, id, ID_LEN) != 0)) {
            printlnfd(Serial, "CONTROLLER #%u ID ERROR", cont_num);
            return false;
        }
    }

    // Verificar msg type
    if (!get_field(2) || (strcmp(field_buff, "4") != 0)) {
        printlnfd(Serial, "CONTROLLER #%u MSG_TYPE ERROR", cont_num);
        return false;
    }
    // Verificar número de fields
    if (msg_fields != n_fields) {
        printlnfd(Serial, "CONTROLLER #%u FIELD_N ERROR, REQUESTED %u, GOT %u:", cont_num, n_fields, msg_fields);
        printlnd(in_buff);
        return false;
    }
    return true;
}

const char* FlowController::get_id() {
    return id;
}

//------------------------------UTILS
bool FlowController::get_field(const char* msg, uint8_t field_i, char* dest, uint8_t max_len) {
    assert(msg);
    assert(dest);
    assert(max_len);

    if (msg[0] == '\0') return false;

    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t field = 0;
    while (1) {
        char c = msg[i];
        if (field == field_i) {
            if (c == '\r' || c == '\0' || c == ',') {
                dest[j] = '\0';
                return true;
            } else {
                dest[j] = c;
                if (++j >= max_len) {
                    return false;
                }
            }
        } else {
            if (c == '\r' || c == '\0') {
                return false;
            } else if (c == ',') {
                field++;
            }
        }
        i++;
    }
    return false;
}

bool FlowController::get_field(uint8_t field_i, char* dest, uint8_t max_len) {
    return get_field(in_buff, field_i, dest, max_len);
}

bool FlowController::get_field(uint8_t field_i) {
    return get_field(in_buff, field_i, field_buff, FIELD_BUFF_LEN);
}