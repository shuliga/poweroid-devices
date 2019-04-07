#include "global.h"
#include "Poweroid10.h"
#include "bluetooth.h"

#define ID "TEST_BT"

Bt BT(ID);

void setup() {
    SSerial.begin(9600);
    SSerial.println("BT TEST init");
    BT.begin();
    SSerial.println("Bluetooth init");
    SSerial.println(BT.remote_on);
    SSerial.println(BT.host);
}

void loop() {
    if (Serial.available()){
        SSerial.write((uint8_t) Serial.read());
    }
    if (SSerial.available()){
        Serial.write(SSerial.read());
    }
    delay(20);
}