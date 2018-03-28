//
// Created by SHL on 25.03.2017.
//

#include "global.h"
#include "relays.h"
#include "pin_io.h"

bool powered[RELAYS];
char *status = (char *) "....";

void Relays::power(uint8_t i, bool _power, bool mapped) {
    if (i < RELAYS) {
        if (i < size() && powered[i] != _power) {
            powered[i] = _power;
#ifndef SSERIAL
            pin_inv(OUT_PINS[i], _power);
#endif
            printRelay(i);
            int8_t mappedIdx = mappings[i];
            if (mapped &&  mappedIdx >= 0) {
                powered[mappedIdx] = _power;
                printRelay((uint8_t) mappedIdx);
            }
        }
    }
}

uint8_t Relays::size() {
    return ARRAY_SIZE(OUT_PINS);
}

char * Relays::relStatus() {
    for(uint8_t i=0; i < RELAYS; i++){
        status[i] = powered[i] ? '@' : '0';
    }
    return status;
}


void Relays::printRelay(uint8_t idx) {
    sprintf(BUFF, REL_FMT, idx, powered[idx] ? REL_POWERED : REL_NOT_POWERED);
#ifdef SSERIAL
    SSerial.println(BUFF);
#else
    Serial.println(BUFF);
#endif
}
