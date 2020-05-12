#include <SoftwareSerial.h>
#include <Wire.h>
#include "global.h"
#include "Poweroid10.h"

#define ID "TEST_BT"

TimingState countdown(250);

static uint8_t current_out = 0;

void setup() {
    for(uint8_t i = 0; i < 4; i++){
        pinMode(INA_PINS[i], );
        digitalPinToAnalogPin(1)
    }
    digitalWrite(INA_PINS[current_out], HIGH);
}

void loop() {
    if(countdown.ping()){
        digitalWrite(INA_PINS[current_out], LOW);
        current_out++;
        current_out == 3 ? 0 : current_out;
        digitalWrite(INA_PINS[current_out], HIGH);
    }
}