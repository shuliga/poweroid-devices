//
// Created by SHL on 09.09.2018.
//

#ifndef INDICATORS_H
#define INDICATORS_H

#include "global.h"
#include "commons.h"
#include "timings.h"

#define IND_1 0
#define IND_2 1
#define IND_3 2

class Indicators{
public:

    static const uint8_t INDICATOR_PINS[];

    void init();
    void set(uint8_t i, bool on);
    void flash(uint8_t i, TimingState *ts, bool trigger);
};

extern Indicators INDICATORS;

#endif //INDICATORS_H
