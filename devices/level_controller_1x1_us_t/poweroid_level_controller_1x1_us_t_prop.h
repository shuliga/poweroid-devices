#ifndef PROPS_H
#define PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = 0;

    Property FACTORY[6] = {

            //val      minV     maxV       scale
            {200L,  50L,   300L,   1L},
            {50L,   30L,   150L,   1L},
            {60000L,60000L,900000L,60000L},
            {60000L,60000L,900000L,60000L},
            {12L,  1L,  23L,  1L},
            {12L,  1L,  23L,  1L}
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// To save program RAM space using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Water level max");
        FACTORY[0].measure = MEASURE_CM;

        FACTORY[1].desc = F("Water level min");
        FACTORY[1].measure = MEASURE_CM;

        FACTORY[2].desc = F("Low water check");
        FACTORY[2].measure = MEASURE_MIN;

        FACTORY[3].desc = F("Full basin check");
        FACTORY[3].measure = MEASURE_MIN;

        FACTORY[4].desc = F("Timer START");
        FACTORY[4].measure = MEASURE_HOUR;

        FACTORY[5].desc = F("Timer STOP");
        FACTORY[5].measure = MEASURE_HOUR;
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;

#endif
