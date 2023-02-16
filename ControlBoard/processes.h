#ifndef PROCESSES_H
#define PROCESSES_H
#include "Arduino.h"

typedef void (*process_fun_t)(void* obj);
#define MAX_PROCESS_FUNS 10

class Processes {
   public:
    static void add_process_fun(process_fun_t fun, void* obj);
    static void add_process_fun(process_fun_t fun);
    static void process_all();
    static void yield();
    static void delay(uint32_t ms);
    static void enter_critical();
    static void exit_critical();

   private:
    typedef struct {
        process_fun_t fun_p;
        bool flag;
        void* obj;
    } fun_data_t;

    static fun_data_t functions[MAX_PROCESS_FUNS];
    static uint8_t fun_i;
    static uint8_t critical;
    static bool print_dt;
};

#endif