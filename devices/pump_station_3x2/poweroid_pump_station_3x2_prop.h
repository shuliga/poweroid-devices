#ifndef PROPS_H
#define PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = -1;

    Property FACTORY[10] = {

            //val      minV     maxV       scale
            {200L,     50L,     300L,     1L},
            {100L,     5L,     150L,     1L},
            {250L,      100L,    400L,     1L},
            {5000L,  3000L,  30000L,  1000L},
            {7200000L, 3600000L,28800000, 3600000L},
            {15000L,   5000L,   30000L,   1000L},
            {15L,   0L,   50L,   1},
            {300000L,  60000L,  1800000L,  60000L},
            {43200000L,  0L,  82800000L,  3600000L},
            {0L,  0L,  3540000L,  60000L},
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// To save program RAM space using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Water level max");
        FACTORY[0].measure = MEASURE_CM;

        FACTORY[1].desc = F("P-out MIN");
        FACTORY[1].measure = MEASURE_KPA;

        FACTORY[2].desc = F("P-out MAX");
        FACTORY[2].measure = MEASURE_KPA;

        FACTORY[3].desc = F("Pump pre-up");
        FACTORY[3].measure = MEASURE_SEC;

        FACTORY[4].desc = F("Pump switch");
        FACTORY[4].measure = MEASURE_HOUR;

        FACTORY[5].desc = F("Pump alarm");
        FACTORY[5].measure = MEASURE_SEC;

        FACTORY[6].desc = F("Low press. warn.");
        FACTORY[6].measure = MEASURE_PERCENT;

        FACTORY[7].desc = F("Low water check");
        FACTORY[7].measure = MEASURE_MIN;

        FACTORY[8].desc = F("Timer");
        FACTORY[8].measure = MEASURE_HOUR;

        FACTORY[9].desc = F("Timer");
        FACTORY[9].measure = MEASURE_MIN;

    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;

#endif
