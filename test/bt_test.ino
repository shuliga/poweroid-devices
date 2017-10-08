#include <Arduino.h>
#include "PoweroidSdk10.h"

#define TX_SS 2
#define RX_SS 3


static const char *name = "PWR-FLR-PASS-1";

Bt *bt;

TimingState ts1 = TimingState(500);
TimingState ts2 = TimingState(250);
TimingState ts3 = TimingState(1000);
MultiClick encoderClick = MultiClick(ENC_BTN_PIN);

Bt BT(name);


void setup() {
    Serial.begin(9600);
    BT.begin();
}

void loop() {
    ts3.flash(8, true);
    McEvent event = encoderClick.checkButton();
    switch (event){
        case DOUBLE_CLICK: digitalWrite(10, HIGH);
            break;
        case CLICK: digitalWrite(10, LOW);
            break;
        case PRESSED: digitalWrite(9, HIGH);
            break;
        case RELEASED: digitalWrite(9, LOW);
            break;
    }


}