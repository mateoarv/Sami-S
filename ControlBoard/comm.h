#ifndef INC_COMM_H_
#define INC_COMM_H_

#include "includes.h"

#define COMM_MASTER_ID 0
#define COMM_MAX_DATA_LEN 16
#define COMM_BUFF_LEN (7 + COMM_MAX_DATA_LEN)
#define COMM_START_CHAR '<'
#define COMM_END_CHAR '>'
#define COMM_MAX_BYTE_DT 5  // mseg
#define COMM_RESP_TO 20     // mseg

#define COMM_MSG_TYPE_RESPONSE 0
#define COMM_MSG_TYPE_ERROR 1
#define COMM_MSG_TYPE_PING 2
#define COMM_MSG_TYPE_READ_VAL 3
#define COMM_MSG_TYPE_DIRECT 4
#define COMM_MSG_TYPE_O2_LOW_CAL 5
#define COMM_MSG_TYPE_O2_HIGH_CAL 6
#define COMM_MSG_TYPE_CO2_LOW_CAL 7

#define COMM_ERROR_ID_UNKNOWN 0
#define COMM_ERROR_ID_CRC 1
#define COMM_ERROR_ID_FORMAT 2
#define COMM_ERROR_ID_OTHER 3

typedef struct {
    uint8_t sender_id;
    uint8_t receiver_id;
    uint8_t msg_type;
    uint8_t data_len;
    uint8_t data[COMM_MAX_DATA_LEN];
} comm_cmd_t;

class Comm {
   public:
    static void send(uint8_t receiver_id, uint8_t msg_type, const uint8_t *data, uint8_t data_len);
    static void send(uint8_t receiver_id, uint8_t msg_type, uint8_t data);
    static void send(uint8_t receiver_id, uint8_t msg_type);
    static bool feed_c(char c);
    static bool get_cmd(comm_cmd_t &dest);
    static bool peek_cmd(comm_cmd_t &dest);
    static bool is_cmd_available();
    static void process(void *obj);

   private:
    static comm_cmd_t cmd;
    static bool cmd_available;
    static uint8_t error_flags;
    static uint32_t last_msg_t;
};

#endif /* INC_COMM_H_ */
