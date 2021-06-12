#ifndef PROPS_H
#define PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

static struct Properties {

    int8_t DEFAULT_PROPERTY = 0;

    static const uint8_t props_size = 5;

    Property FACTORY[props_size] = {

            //val      minV     maxV       scale
            {200L,  50L,   300L,   1L},
            {50L,   30L,   150L,   1L},
            {8L,  0L,  23L,  1L},
            {0L,  0L,  59L,  1L},
            {60000L,60000L,900000L,60000L},
    };

// Property strings should be stored in FlashRAM
// To save program RAM space using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Water level max");
        FACTORY[0].measure = MEASURE_CM;

        FACTORY[1].desc = F("Water level min");
        FACTORY[1].measure = MEASURE_CM;

        FACTORY[2].desc = F("Timer hrs.");
        FACTORY[2].measure = MEASURE_HOUR;

        FACTORY[3].desc = F("Timer min.");
        FACTORY[3].measure = MEASURE_MIN;

        FACTORY[4].desc = F("Level check");
        FACTORY[4].measure = MEASURE_MIN;
    }

    void factoryReset() {
        for (int i = 0; i < props_size; i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;

#endif
