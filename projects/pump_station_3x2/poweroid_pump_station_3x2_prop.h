#ifndef PROPS_H
#define PROPS_H

#include <../../Poweroid_SDK_10/src/properties.h>
#include <../../Poweroid_SDK_10/src/commons.h>
#include <../../Poweroid_SDK_10/src/timings.h>

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = 1;

    Property FACTORY[7] = {

            //val      minV     maxV       scale
            {200L,     50L,     300L,     1L},
            {100L,     50L,     150L,     1L},
            {250L,      100L,    400L,     1L},
            {15000L,   5000L,   30000L,   1000L},
            {7200000L, 3600000L,28800000, 3600000L},
            {900000L,  300000L, 1800000L, 60000L},
            {120000L,  60000L,  600000L,  60000L}
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Water level max");
        FACTORY[0].measure = CM;

        FACTORY[1].desc = F("P-out MIN");
        FACTORY[1].measure = KPA;

        FACTORY[2].desc = F("P-out MAX");
        FACTORY[2].measure = KPA;

        FACTORY[3].desc = F("Pump timeout");
        FACTORY[3].measure = SEC;

        FACTORY[4].desc = F("Pump switch");
        FACTORY[4].measure = HOUR;

        FACTORY[5].desc = F("Pump alarm");
        FACTORY[5].measure = SEC;

        FACTORY[6].desc = F("Pump pre-up");
        FACTORY[6].measure = MIN;
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;

#endif
