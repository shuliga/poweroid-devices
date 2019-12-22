#ifndef PROPS_H
#define PROPS_H

#include <../../Poweroid_SDK_10/src/properties.h>
#include <../../Poweroid_SDK_10/src/commons.h>
#include <../../Poweroid_SDK_10/src/timings.h>

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = 1;

    Property FACTORY[4] = {

            //val      minV       maxV        scale
            {-40L,      -99L,     -25L,       1L},
            {-15L,      -24L,     0L,         1L},
            {36000000L, 3600000L, 180000000L, 3600000L},
            {36000000L, 3600000L, 180000000L, 3600000L}
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("P-tens min");
        FACTORY[0].measure = KPA;

        FACTORY[1].desc = F("P-tens max");
        FACTORY[1].measure = KPA;

        FACTORY[2].desc = F("Fill time");
        FACTORY[2].measure = HOUR;

        FACTORY[3].desc = F("Alarm time-out");
        FACTORY[3].measure = HOUR;
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;

#endif
