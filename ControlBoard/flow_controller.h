#ifndef FLOW_CONTROLLER_H
#define FLOW_CONTROLLER_H

#include "includes.h"

class FlowController {
   private:
    static const uint8_t ID_LEN = 5;
    static const uint8_t ID_BUFF_LEN = ID_LEN + 1;
    static const uint8_t IN_BUFF_LEN = 100;
    static const uint8_t AUX_BUFF_LEN = 50;
    static const uint8_t FIELD_BUFF_LEN = 50;

   public:
    enum class measure_unit_e : uint8_t {
        ML = 0,
        L = 3,
        SL = 19
    };
    enum class time_unit_e : uint8_t {
        NONE = 0,
        SEC,
        MIN,
        HRS,
        DAY
    };

    FlowController(Stream& _serial, uint8_t _cont_num) : serial(_serial),
                                                         cont_num(_cont_num){};
    bool init();
    void process();
    static void static_process(void* obj);
    const char* get_id();

    bool read_SP_rate(uint8_t channel, float& rate);
    bool write_SP_rate_ATPD(uint8_t channel, float rate_ATPD);
    bool write_SP_rate_STPD0(uint8_t channel, float rate_STPD0);

   private:
    Stream& serial;
    char id[ID_BUFF_LEN];
    char in_buff[IN_BUFF_LEN];
    char field_buff[FIELD_BUFF_LEN];
    char aux_buff[AUX_BUFF_LEN];
    uint8_t in_buff_i = 0;
    bool msg_pending = false;
    uint8_t msg_fields = 0;
    bool is_init = false;
    bool has_id = false;
    const uint8_t cont_num;

    bool feed_c(char c);
    bool wait_response(uint8_t n_fields);

    bool cmd_id();

    bool cmd_read_value(uint8_t channel, bool input_port, uint8_t param);
    bool cmd_read_value(uint8_t channel, bool input_port, uint8_t param, uint32_t& dest);
    bool cmd_read_value(uint8_t channel, bool input_port, uint8_t param, float& dest);
    bool cmd_write_value(uint8_t channel, bool input_port, uint8_t param, const char* value);
    bool cmd_write_value(uint8_t channel, bool input_port, uint8_t param, uint32_t value);
    bool cmd_write_value(uint8_t channel, bool input_port, uint8_t param, float value);
    bool cmd_read_measure_unit(uint8_t channel, measure_unit_e& dest);
    bool cmd_write_measure_unit(uint8_t channel, measure_unit_e unit);
    bool cmd_read_time_unit(uint8_t channel, time_unit_e& dest);
    bool cmd_write_time_unit(uint8_t channel, time_unit_e unit);

    static bool get_field(const char* msg, uint8_t field_i, char* dest, uint8_t max_len);
    bool get_field(uint8_t field_i, char* dest, uint8_t max_len);
    bool get_field(uint8_t field_i);
};

#endif