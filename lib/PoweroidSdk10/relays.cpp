//
// Created by SHL on 25.03.2017.
//

#include "global.h"
#include "relays.h"

static bool powered[RELAYS];
static unsigned char status[5] = {'.', '.', '.', '.', '\0'};

void Relays::power(uint8_t i, bool _power) {
    if (i < RELAYS)
    {
        if (i < size() && powered[i] != _power)
        {
            powered[i] = _power;
#ifndef SSERIAL
            digitalWrite(OUT_PINS[i], _power ? LOW : HIGH);
#endif
            printRelay(i);
            int8_t mappedIdx = mappings[i];
            if (mapped &&  mappedIdx >= 0)
            {
                powered[mappedIdx] = _power;
                printRelay((uint8_t) mappedIdx);
            }
        }
    }
}

uint8_t Relays::size() {
    return ARRAY_SIZE(OUT_PINS);
}

unsigned char * Relays::relStatus() {
    const uint8_t r_size = mapped ? RELAYS : size();
    for(uint8_t i = 0; i < r_size; ++i)
    {
        status[i] = (powered[i] ? (unsigned char) 128 : (unsigned char)127);
    }
    return status;
}

void Relays::reset(){
    for(uint8_t i=0; i < size(); ++i){
        digitalWrite(OUT_PINS[i], HIGH);
    }
}

void Relays::printRelay(uint8_t idx) {
    sprintf(BUFF, REL_FMT, idx, powered[idx] ? REL_POWERED : REL_NOT_POWERED);
#ifdef SSERIAL
    SSerial.println(BUFF);
#else
    Serial.println(BUFF);
#endif
}
