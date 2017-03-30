//
// Created by SHL on 25.03.2017.
//

#include "Relays.h"
#include "pin_io.h"

void Relays::powerOn(uint8_t i) {
    power(OUT_PINS[i], true);
}

void Relays::powerOff(uint8_t i) {
    power(OUT_PINS[i], false);
}

void Relays::power(uint8_t i, bool _power) {
    pin_inv(OUT_PINS[i], _power);
}

uint8_t Relays::size() {
    return ARRAY_SIZE(OUT_PINS);
}
