#include <Arduino.h>

#define REL_A 8
#define REL_B 9

static bool blink = false;

void setup() {
    Serial.begin(9600);
    Serial.println("setup()");
    pinMode(LED, OUTPUT);
    pinMode(REL_A, OUTPUT);
    pinMode(REL_B, OUTPUT);
}

void loop() {
    Serial.println("loop()");
    digitalWrite(LED, blink =! blink);
    digitalWrite(REL_A, blink);
    digitalWrite(REL_B, !blink);
    delay(1000);
}
