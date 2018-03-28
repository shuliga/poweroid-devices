//
// Created by SHL on 25.03.2017.
//

#ifndef POWEROID_10_RELAYS_H
#define POWEROID_10_RELAYS_H

#include "commons.h"

#define REL_PREFIX "Rel["
#define REL_FMT REL_PREFIX "%i]: %s"

const char *const REL_POWERED = "ON";
const char *const REL_NOT_POWERED = "OFF";

const uint8_t OUT_PINS[] = {PWR1_PIN, PWR2_PIN};
static const uint8_t VIRTUAL_RELAYS = 2;
static const uint8_t RELAYS = ARRAY_SIZE(OUT_PINS) + VIRTUAL_RELAYS;

class Relays {
public:
    int8_t mappings[VIRTUAL_RELAYS] = {2, 3};

    void power(uint8_t i, bool _power, bool mapped);

    uint8_t size();

    void printRelay(uint8_t idx);

    char *relStatus();
};


#endif //POWEROID_10_RELAYS_H
