#ifndef FAN_PROPS_H
#define FAN_PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

#define GET_PROP_NORM(i) FAN_PROPS.FACTORY[(i)].runtime / FAN_PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = 6;

    Property FACTORY[8] = {

            //val      minV     maxV       scale
            {600000L,  60000L,  36000000L, 60000L},
            {5000L,    1000L,   3600000L,  1000L},
            {60000L,   1000L,   3600000L,  1000L},
            {70L,      50L,     99L,       1L},
            {1800000L, 600000L, 36000000L, 60000L},
            {300000L,  600000L, 36000000L, 60000L},
            {26L,      6L,      35L,       1L},
            {300000L,  60000L,  3600000L,  60000L}

    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Fan run time");
        FACTORY[0].measure = 3;

        FACTORY[1].desc = F("Fan delay");
        FACTORY[1].measure = 4;

        FACTORY[2].desc = F("Light stand-by");
        FACTORY[2].measure = 4;

        FACTORY[3].desc = F("Humidity threshold");
        FACTORY[3].measure = 6;

        FACTORY[4].desc = F("Humidity fan run time");
        FACTORY[4].measure = 3;

        FACTORY[5].desc = F("Humidity delay");
        FACTORY[5].measure = 3;

        FACTORY[6].desc = F("Temperature threshold");
        FACTORY[6].measure = 5;

        FACTORY[7].desc = F("Temperature delay");
        FACTORY[7].measure = 3;
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} FAN_PROPS;

#endif
