#include <properties.h>
#include <commons.h>
#include <timings.h>

static struct Properties {

    int8_t DEFAULT_PROPERTY = 0;

    Property FACTORY[1] = {
            //val      minV      maxV      scale
            {12L,      0L,       24L,      1L},
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {

        factoryReset();

        FACTORY[0].desc = F("Day circle");
        FACTORY[0].measure = MEASURE_HOUR;

    }

    void factoryReset() {
        for (uint8_t i = 0; i < props_size; i++) {
            FACTORY[i].runtime = FACTORY[i].val;
        }
    }

} PROPS;
