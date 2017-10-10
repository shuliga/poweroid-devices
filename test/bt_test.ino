#include <Arduino.h>
#include <SoftwareSerial.h>
#include "PoweroidSdk10.h"

#define TX_SS 2
#define RX_SS 3


static const char *name = "PWR-FLR-PASS-1";

Bt BT(name);

void setup() {
    Serial.begin(9600);
    BT.begin();
    BT.SSerial.println(BT.isConnected());
    BT.SSerial.println(BT.getPassive());
}

void loop() {
    if (Serial.available()){
        BT.SSerial.write((uint8_t) Serial.read());
    }
    if (BT.SSerial.available()){
        Serial.write(BT.SSerial.read());
    }
    delay(20);
}