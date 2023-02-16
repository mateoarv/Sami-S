#include "includes.h"
#include "Wire.h"
#include "program.h"

void setup() {
    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);
    pinMode(PIN_BUZZ, OUTPUT);
    pinMode(PIN_I2C_RST, OUTPUT);
    pinMode(PIN_SEND_EN, OUTPUT);
    pinMode(PIN_MOT_1, OUTPUT);
    pinMode(PIN_MOT_2, OUTPUT);
    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    pinMode(PIN_BTN_3, INPUT_PULLUP);
    pinMode(PIN_P1, INPUT);
    pinMode(PIN_P2, INPUT);
    analogWriteFrequency(PIN_BUZZ, 3000);
    analogWriteFrequency(PIN_MOT_1, 1000);
    analogWriteFrequency(PIN_MOT_2, 1000);
    analogReadResolution(13);
    Serial.begin(115200);
    SerialExt.begin(19200);
    SerialComm.begin(115200);
    SerialCont1.begin(9600);
    SerialCont2.begin(9600);
    SerialCont3.begin(9600);

    Wire1.setSDA(PIN_SDA);
    Wire1.setSCL(PIN_SCL);
    Wire1.begin();

    Serial.println("RESET");

    digitalWrite(PIN_LED_1, HIGH);
    digitalWrite(PIN_LED_2, HIGH);
    digitalWrite(PIN_LED_3, HIGH);
    analogWrite(PIN_BUZZ, 128);
    delay(100);
    digitalWrite(PIN_LED_1, LOW);
    digitalWrite(PIN_LED_2, LOW);
    digitalWrite(PIN_LED_3, LOW);
    analogWrite(PIN_BUZZ, 0);
    delay(3000);

    Program::start();
}

void loop() {
    Processes::process_all();
}