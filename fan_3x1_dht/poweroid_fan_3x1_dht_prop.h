#ifndef FAN_STATE_H
#define FAN_STATE_H

#include "properties.h"
#include "commons.h"
#include "timings.h"

#define GET_PROP_NORM(i) FAN_PROPS.RUNTIME[(i)] / FAN_PROPS.FACTORY[(i)].scale

static struct Properties {

    Property FACTORY[8] = {

            //desc  value   minV    maxV        scale
            {NULL, 600000L,  60000L,  36000000L, 60000L},
            {NULL, 5000L,    1000L,   3600000L,  1000L},
            {NULL, 60000L,   1000L,   3600000L,  1000L},
            {NULL, 70L,      50L,     99L,       1L},
            {NULL, 1800000L, 600000L, 36000000L, 60000L},
            {NULL, 300000L,  600000L, 36000000L, 60000L},
            {NULL, 26L,      6L,      35L,       1L},
            {NULL, 300000L,  60000L,  3600000L,  60000L}

    };

    long RUNTIME[ARRAY_SIZE(FACTORY)];

// Declaring alternate array Property*[2][4] that represents 2 sets (per each state) of 4 properties
// This makes possible state - related iterations over properties
//    long (*_RUNTIME)[4] = (long (*)[4]) &RUNTIME;

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Fan run time (min)");
        FACTORY[1].desc = F("Fan delay (sec)");
        FACTORY[2].desc = F("Light stand-by (sec)");
        FACTORY[3].desc = F("Humidity threshold (%)");
        FACTORY[4].desc = F("Humidity fan run time (min)");
        FACTORY[5].desc = F("Humidity delay (min)");
        FACTORY[6].desc = F("Temperature threshold (~C)");
        FACTORY[7].desc = F("Temperature delay (min)");
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            RUNTIME[i] = FACTORY[i].val;
        }
    }

} FAN_PROPS;

#endif
