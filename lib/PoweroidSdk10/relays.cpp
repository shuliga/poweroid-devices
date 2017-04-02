//
// Created by SHL on 25.03.2017.
//

#include "relays.h"
#include "pin_io.h"

static bool powered[ARRAY_SIZE(OUT_PINS)];
static char CHAR_BUF[32];

void Relays::powerOn(uint8_t i) {
    power(OUT_PINS[i], true);
}

void Relays::powerOff(uint8_t i) {
    power(OUT_PINS[i], false);
}

void Relays::power(uint8_t i, bool _power) {
    if (i < ARRAY_SIZE(OUT_PINS)){
        pin_inv(OUT_PINS[i], _power);
        powered[i] = _power;
    }
}

uint8_t Relays::size() {
    return ARRAY_SIZE(OUT_PINS);
}

const char* Relays::printRelay(uint8_t idx) {
    sprintf(CHAR_BUF, "Relay[%i]: %s", idx, powered[idx] ? "powered" : "not powered");
    return CHAR_BUF;
}
