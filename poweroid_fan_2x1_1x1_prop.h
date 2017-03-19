#ifndef FAN_PROPS_H
#define FAN_PROPS_H

#include "properties.h"
#include "commons.h"
#include "timings.h"

struct Timings {
    TimingState countdown_power, countdown_light, timeAfter_lightOff;
};


static struct Properties {

    Property FACTORY[8] = {

            //desc  value   minV    maxV        scale
            {NULL, 600000L, 60000L, 36000000L, 60000L},
            {NULL, 5000L,   1000,   3600000L,  1000},
            {NULL, 60000L,  1000,   3600000L,  1000},
            {NULL, 60000L,  1000,   3600000L,  1000},

            {NULL, 600000L, 60000L, 36000000L, 60000L},
            {NULL, 5000L,   1000,   3600000L,  1000},
            {NULL, 60000L,  1000,   3600000L,  1000},
            {NULL, 60000L,  1000,   3600000L,  1000}

    };

    long RUNTIME[ARRAY_SIZE(FACTORY)];

// Declaring alternate array Property*[2][4] that represents 2 sets (per each state) of 4 properties
// This makes possible state - related iterations
    long (*_RUNTIME)[4] = (long (*)[4]) &RUNTIME;

    int size_t = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("POWER-1 run time (min)");
        FACTORY[1].desc = F("POWER-1 delay (sec)");
        FACTORY[2].desc = F("LIGHT-1.1 stand-by (sec)");
        FACTORY[3].desc = F("LIGHT-1.x stand-by (sec)");

        FACTORY[4].desc = F("POWER-2 run time (min)");
        FACTORY[5].desc = F("POWER-2 delay (sec)");
        FACTORY[6].desc = F("LIGHT-2.1 stand-by (sec)");
        FACTORY[7].desc = F("LIGHT-2.2 stand-by (sec)");
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            RUNTIME[i] = FACTORY[i].val;
        }
    }

} FAN_PROPS;

#endif
