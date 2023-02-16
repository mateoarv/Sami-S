#include "processes.h"
#include "error.h"
#include "utils.h"
Processes::fun_data_t Processes::functions[MAX_PROCESS_FUNS] = {};
uint8_t Processes::fun_i = 0;
uint8_t Processes::critical = 0;
bool Processes::print_dt = false;

void Processes::add_process_fun(process_fun_t fun, void *obj) {
    assert(fun_i < MAX_PROCESS_FUNS);
    functions[fun_i].fun_p = fun;
    functions[fun_i].flag = false;
    functions[fun_i].obj = obj;
    fun_i++;
}

void Processes::add_process_fun(process_fun_t fun) {
    add_process_fun(fun, NULL);
}

void Processes::process_all() {
    static uint16_t entries = 0;
    if (critical) {
        return;
    }
    entries++;
    const bool print = (entries == 1) && print_dt;
    uint32_t t0 = 0;
    uint32_t t1 = 0;
    if (print) {
        t0 = Utils::tick();
        printlnd();
    }
    for (uint8_t i = 0; i < fun_i; i++) {
        if (!functions[i].flag) {
            functions[i].flag = true;
            if (print) t1 = Utils::tick();
            (functions[i].fun_p)(functions[i].obj);
            if (print) {
                const uint32_t dt = Utils::tock(t1, false);
                printlnfd(Serial, "Process #%u dt: %u", i, dt);
            }
            functions[i].flag = false;
        }
    }
    if (print) Utils::tock(t0, "Total process dt: ");
    entries--;
}

void Processes::yield() {
    process_all();
}

void Processes::delay(uint32_t ms) {
    uint32_t t0 = millis();
    while (millis() - t0 < ms) {
        process_all();
    }
    uint32_t extra_delay = millis() - t0 - ms;
    if (extra_delay > 0) {
        // printlnfd(Serial, "Extra delay: %u", extra_delay);
    }
}

void Processes::enter_critical() {
    critical++;
}
void Processes::exit_critical() {
    critical--;
}