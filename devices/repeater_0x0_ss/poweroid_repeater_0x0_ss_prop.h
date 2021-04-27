#ifndef PROPS_H
#define PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = 1;

    Property FACTORY[2] = {

            //val             minV       maxV            scale
            {9600L,      1200L,     115200L,       1L},
            {9600L,      1200L,     115200L,       1L}
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("A-in baud");
        FACTORY[0].measure = MEASURE_BAUD;

        FACTORY[1].desc = F("B-in baud");
        FACTORY[1].measure = MEASURE_BAUD;

    }

    void factoryReset() {
        for (auto & i : FACTORY) {
            i.runtime = i.val;
        }
    }

} PROPS;

#endif
