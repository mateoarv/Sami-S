#ifndef UI_H
#define UI_H
#include "Arduino.h"

class UI {
   public:
    static void init();
    static void buzz(uint8_t n, uint16_t dt);
    static void buzz(uint8_t n, uint16_t dt, uint8_t volume);
    static void buzz_loud(uint8_t n, uint16_t dt);
    static bool get_btn_1_pressed();
    static bool get_btn_2_pressed();
    static bool get_btn_3_pressed();

   private:
    static volatile bool btn_1_pressed;
    static volatile bool btn_2_pressed;
    static volatile bool btn_3_pressed;
    static void isr_btn_1();
    static void isr_btn_2();
    static void isr_btn_3();
};

#endif