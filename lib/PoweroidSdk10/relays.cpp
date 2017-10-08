//
// Created by SHL on 25.03.2017.
//

#include "relays.h"
#include "pin_io.h"

static bool powered[RELAYS];
static char CHAR_BUF[32];

void Relays::powerOn(uint8_t i) {
    return power(i, true);
}

void Relays::powerOff(uint8_t i) {
    return power(i, false);
}

void Relays::power(uint8_t i, bool _power) {
    if (i < RELAYS) {
        if (i < ARRAY_SIZE(OUT_PINS) && powered[i] != _power) {
            powered[i] = _power;
            pin_inv(OUT_PINS[i], _power);
            printRelay(i);
        }
    }
}

uint8_t Relays::size() {
    return ARRAY_SIZE(OUT_PINS);
}

void Relays::printRelay(uint8_t idx) {
    sprintf(CHAR_BUF, REL_FMT, idx, powered[idx] ? REL_POWERED : REL_NOT_POWERED);
    Serial.println(CHAR_BUF);
}
