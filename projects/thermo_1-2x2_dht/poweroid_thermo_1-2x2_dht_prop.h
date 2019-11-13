#ifndef FAN_PROPS_H
#define FAN_PROPS_H

#include <../../Poweroid_SDK_10/src/properties.h>
#include <../../Poweroid_SDK_10/src/commons.h>
#include <../../Poweroid_SDK_10/src/timings.h>

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

static struct Properties {

    int8_t DEFAULT_PROPERTY = 2;

    Property FACTORY[7] = {

            //val      minV      maxV      scale
            {24L,      6L,       35L,      1L},
            {21L,      6L,       35L,      1L},
            {18L,      6L,       35L,      1L},
            {15L,      6L,       35L,      1L},
            {8L,       6L,       18L,      1L},
            {10800000L,3600000L, 43200000L,3600000L},
            {900000L, 300000L,  7200000L, 60000L}

    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {

        factoryReset();

        FACTORY[0].desc = F("Floor t.");
        FACTORY[0].measure = DEG_C;

        FACTORY[1].desc = F("Heater t.");
        FACTORY[1].measure = DEG_C;

        FACTORY[2].desc = F("Floor t. ECO");
        FACTORY[2].measure = DEG_C;

        FACTORY[3].desc = F("Heater t. ECO");
        FACTORY[3].measure = DEG_C;

        FACTORY[4].desc = F("Away t.");
        FACTORY[4].measure = DEG_C;

        FACTORY[5].desc = F("Floor sw delay");
        FACTORY[5].measure = HOUR;

        FACTORY[6].desc = F("Heater sw delay");
        FACTORY[6].measure = MIN;

    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;

#endif
