#include "UI.h"
#include "PinDef.h"

volatile bool UI::btn_1_pressed = false;
volatile bool UI::btn_2_pressed = false;
volatile bool UI::btn_3_pressed = false;
#define MAX_DUTY 205

void UI::init() {
    attachInterrupt(PIN_BTN_1, isr_btn_1, FALLING);
    attachInterrupt(PIN_BTN_2, isr_btn_2, FALLING);
    attachInterrupt(PIN_BTN_3, isr_btn_3, FALLING);
}

void UI::buzz(uint8_t n, uint16_t dt, uint8_t volume) {
    volume = min(volume, 100);
    for (uint8_t i = 0; i < n; i++) {
        analogWrite(PIN_BUZZ, map(volume, 0, 100, 0, MAX_DUTY));
        delay(dt);
        analogWrite(PIN_BUZZ, 0);
        if (i + 1 < n) {
            delay(dt);
        }
    }
}

void UI::buzz(uint8_t n, uint16_t dt) {
    buzz(n, dt, 50);
}

void UI::buzz_loud(uint8_t n, uint16_t dt) {
    buzz(n, dt, 100);
}

void UI::isr_btn_1() {
    SCB_AIRCR = 0x05FA0004;
    btn_1_pressed = true;
}
void UI::isr_btn_2() {
    btn_2_pressed = true;
}
void UI::isr_btn_3() {
    btn_3_pressed = true;
}

bool UI::get_btn_1_pressed() {
    if (btn_1_pressed) {
        btn_1_pressed = false;
        return true;
    }
    return false;
}

bool UI::get_btn_2_pressed() {
    if (btn_2_pressed) {
        btn_2_pressed = false;
        return true;
    }
    return false;
}

bool UI::get_btn_3_pressed() {
    if (btn_3_pressed) {
        btn_3_pressed = false;
        return true;
    }
    return false;
}