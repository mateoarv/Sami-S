#ifndef GLOBAL_VALUES_H
#define GLOBAL_VALUES_H
#include "Arduino.h"

class GlobalValues {
   public:
    static void set_p_atm_bar(float p_atm);
    static float get_p_atm_bar();
    static float get_temp_c();
    static bool get_dbg_msgs_on();
    static void set_dbg_msgs_on(bool on);

   private:
    static float p_atm_bar;
    static bool dbg_msgs_on;
};

#endif