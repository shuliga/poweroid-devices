//
// Created by SHL on 25.03.2017.
//

#ifndef POWEROID_10_RELAYS_H
#define POWEROID_10_RELAYS_H

#include "commons.h"

const uint8_t OUT_PINS[] = {PWR1_PIN, PWR2_PIN};

class Relays {

public:
    Relays(){}

    void powerOn(uint8_t i);
    void powerOff(uint8_t i);
    void power(uint8_t i, bool _power);

    uint8_t size();

    const char * printRelay(uint8_t idx);
};


#endif //POWEROID_10_RELAYS_H
