#include "comm.h"

#define COMM_ERROR_MSG_MISSED 1
#define COMM_ERROR_TIMEOUT 2
#define COMM_ERROR_COLLISION 4
#define COMM_ERROR_INVALID_LEN 8
#define COMM_ERROR_INVALID_CRC 16
#define COMM_ERROR_CORRUPT_MSG 32

bool Comm::cmd_available = false;
comm_cmd_t Comm::cmd = {};
uint8_t Comm::error_flags = 0;
uint32_t Comm::last_msg_t = 0;

void Comm::send(uint8_t receiver_id, uint8_t msg_type, const uint8_t *data, uint8_t data_len) {
    const int32_t dt = 2 - (millis() - last_msg_t);
    if (dt > 0) {
        Processes::delay(dt);
    }
    digitalWrite(PIN_SEND_EN, 1);
    // Processes::delay(1);
    delay(1);
    SerialComm.print(COMM_START_CHAR);
    SerialComm.write(COMM_MASTER_ID);
    SerialComm.write(receiver_id + 1);
    SerialComm.write(msg_type);
    SerialComm.write(data_len);
    if (data_len > 0 && data) {
        SerialComm.write(data, data_len);
    }
    SerialComm.write(0);  // TODO: CRC
    SerialComm.print(COMM_END_CHAR);
    delay(1);
    // delayMicroseconds(500);
    digitalWrite(PIN_SEND_EN, 0);
    last_msg_t = millis();
}

void Comm::send(uint8_t receiver_id, uint8_t msg_type, uint8_t data) {
    send(receiver_id, msg_type, &data, 1);
}

void Comm::send(uint8_t receiver_id, uint8_t msg_type) {
    send(receiver_id, msg_type, NULL, 0);
}

bool Comm::feed_c(char c) {
    static const uint32_t timeout = 10000 / 115200 + COMM_MAX_BYTE_DT;
    static uint8_t state = 0;
    static uint8_t data_i = 0;
    static uint8_t crc = 0;
    static uint32_t t0 = 0;
    if (cmd_available) {
        error_flags |= COMM_ERROR_MSG_MISSED;
    }

    // Verificar timeout
    if (state && (millis() - t0 > timeout)) {
        state = 0;
        error_flags |= COMM_ERROR_TIMEOUT;
    }

    switch (state) {
        case 0:  // Esperando caracter de inicio
            if (c == COMM_START_CHAR) {
                data_i = 0;
                cmd_available = false;
                state++;
            }
            break;
        case 1:  // Esperando ID de sender
            cmd.sender_id = c;
            if (cmd.sender_id == COMM_MASTER_ID) {
                error_flags |= COMM_ERROR_COLLISION;
            } else {
                cmd.sender_id--;
            }
            state++;
            break;
        case 2:  // Esperando ID de receiver
            cmd.receiver_id = c;
            state++;
            break;
        case 3:  // Esperando tipo de mensaje
            cmd.msg_type = c;
            state++;
            break;
        case 4:  // Esperando data_len
            cmd.data_len = c;
            if (cmd.data_len > COMM_MAX_DATA_LEN) {
                error_flags |= COMM_ERROR_INVALID_LEN;
                state = 0;
            } else {
                state = cmd.data_len ? 5 : 6;
            }
            break;
        case 5:  // Esperando data
            cmd.data[data_i] = c;
            if (++data_i >= cmd.data_len) {
                state++;
            }
            break;
        case 6:  // Esperando CRC
            crc = c;
            state++;
            break;
        case 7:  // Esperando caracter de fin
            if (c == COMM_END_CHAR) {
                // TODO: Validar CRC
                cmd_available = true;
                last_msg_t = millis();
            } else {
                error_flags |= COMM_ERROR_CORRUPT_MSG;
            }
            state = 0;
            break;
    }

    if (state) {
        t0 = millis();
    }

    return cmd_available;
}

void Comm::process(void *obj) {
    while (SerialComm.available()) {
        feed_c(SerialComm.read());
    }
    if (error_flags & COMM_ERROR_MSG_MISSED) {
        printlnd("COMM ERROR: Msg missed");
        error_flags &= (~COMM_ERROR_MSG_MISSED);
    }
    if (error_flags & COMM_ERROR_TIMEOUT) {
        printlnd("COMM ERROR: Timeout");
        error_flags &= (~COMM_ERROR_TIMEOUT);
    }
    if (error_flags & COMM_ERROR_COLLISION) {
        printlnd("COMM ERROR: Collision");
        error_flags &= (~COMM_ERROR_COLLISION);
    }
    if (error_flags & COMM_ERROR_INVALID_LEN) {
        printlnd("COMM ERROR: Invalid length");
        error_flags &= (~COMM_ERROR_INVALID_LEN);
    }
    if (error_flags & COMM_ERROR_INVALID_CRC) {
        printlnd("COMM ERROR: Invalid CRC");
        error_flags &= (~COMM_ERROR_INVALID_CRC);
    }
    if (error_flags & COMM_ERROR_CORRUPT_MSG) {
        printlnd("COMM ERROR: Corrupt msg");
        error_flags &= (~COMM_ERROR_CORRUPT_MSG);
    }
    if (error_flags) {
        printlnfd(Serial, "UNKNOWN COMM ERRORS: %b\n", error_flags);
    }
}

bool Comm::get_cmd(comm_cmd_t &dest) {
    if (cmd_available) {
        dest = cmd;
        cmd_available = false;
        return true;
    }
    return false;
}
bool Comm::peek_cmd(comm_cmd_t &dest) {
    if (cmd_available) {
        dest = cmd;
        return true;
    }
    return false;
}
bool Comm::is_cmd_available() {
    return cmd_available;
}
