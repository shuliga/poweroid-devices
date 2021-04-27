#include <properties.h>
#include <commons.h>
#include <timings.h>

static struct Properties {

    int8_t DEFAULT_PROPERTY = 1;

    Property FACTORY[8] = {

            //val      minV      maxV      scale
            {24L,      6L,       35L,      1L},
            {21L,      6L,       35L,      1L},
            {18L,      6L,       35L,      1L},
            {15L,      6L,       35L,      1L},
            {8L,       6L,       18L,      1L},
            {10800000L,3600000L, 43200000L,3600000L},
            {900000L, 300000L,  7200000L, 60000L},
//            {0L,      -25L,     25L,       1},
            {0L,      -25L,     25L,       1}

    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {

        factoryReset();

        FACTORY[0].desc = F("Floor t.");
        FACTORY[0].measure = MEASURE_DEG_C;

        FACTORY[1].desc = F("Heater t.");
        FACTORY[1].measure = MEASURE_DEG_C;

        FACTORY[2].desc = F("Floor t. ECO");
        FACTORY[2].measure = MEASURE_DEG_C;

        FACTORY[3].desc = F("Heater t. ECO");
        FACTORY[3].measure = MEASURE_DEG_C;

        FACTORY[4].desc = F("Away t.");
        FACTORY[4].measure = MEASURE_DEG_C;

        FACTORY[5].desc = F("Floor sw delay");
        FACTORY[5].measure = MEASURE_HOUR;

        FACTORY[6].desc = F("Heater sw delay");
        FACTORY[6].measure = MEASURE_MIN;

        FACTORY[7].desc = F("Temp. correction");
        FACTORY[7].measure = MEASURE_PERCENT;

//        FACTORY[8].desc = F("Humid correction");
//        FACTORY[8].measure = MEASURE_PERCENT;

    }

    void factoryReset() {
        for (uint8_t i = 0; i < props_size; i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;
