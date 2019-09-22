//
// Created by SHL on 09.09.2018.
//

#include "indicators.h"

Indicators INDICATORS;

const uint8_t Indicators::INDICATOR_PINS[] = {IND1_PIN, IND2_PIN, IND3_PIN};

void Indicators::init() {
    for(uint8_t i = 0; i < ARRAY_SIZE(INDICATOR_PINS); ++i) {
        pinMode(INDICATOR_PINS[i], OUTPUT);
        digitalWrite(INDICATOR_PINS[i], LOW);
    }
}

void Indicators::set(uint8_t i, bool on) {
    digitalWrite(INDICATOR_PINS[i], on ? HIGH : LOW);
}

void Indicators::flash(uint8_t i, bool flash, bool trigger) {
    digitalWrite(INDICATOR_PINS[i], trigger ? (flash ? HIGH : LOW) : LOW);
}
