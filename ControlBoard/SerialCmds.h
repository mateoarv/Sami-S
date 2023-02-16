#ifndef SERIAL_CMDS_H
#define SERIAL_CMDS_H
#include "includes.h"

#define SERIAL_BUFF_LEN 100
#define MAX_CMD_FIELDS 10
#define MAX_FIELD_LEN 20

class SerialCmds {
   public:
    static void process(void* obj);
    static bool get_field(char* dest, uint8_t i);
    static bool get_field(float& dest, uint8_t i);
    static bool get_field(uint8_t& dest, uint8_t i);
    static bool get_field(uint32_t& dest, uint8_t i);
    static bool get_field(int32_t& dest, uint8_t i);
    static bool get_field(bool& dest, uint8_t i);
    static bool cmd_available();
    static void print_cmd();

   private:
    typedef enum {
        STR,
        INT,
        FLOAT
    } field_type_t;
    typedef struct {
        uint8_t i;  // incluyente
        uint8_t j;  // excluyente
        field_type_t type;
    } field_t;

    static char in_buff[SERIAL_BUFF_LEN];
    static uint8_t buff_i;
    static field_t fields[MAX_CMD_FIELDS];
    static uint8_t n_fields;
    static const char* int_pattern;
    static const char* float_pattern;
    static bool cmd_ready;

    static void extract_fields();
    static void check_cmd();
    static void assign_field_type(field_t& field);
};

#endif