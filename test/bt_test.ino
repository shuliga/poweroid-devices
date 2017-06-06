#include <Arduino.h>
#include "PoweroidSdk10.h"

#define TX_SS 2
#define RX_SS 3


static const char *name = "PWR-FLR-PASS-1";

Bt *bt;

TimingState ts1 = TimingState(500);
TimingState ts2 = TimingState(250);
TimingState ts3 = TimingState(1000);

void setup() {
    Serial.begin(9600);
    bt = new Bt(name);
}

void loop() {
//    if (bt->getConnected()){
//        ts1.flash(10, bt->getPassive());
//        ts2.flash(10, !bt->getPassive());
//    } else{
        ts3.flash(8, true);
//    }
}