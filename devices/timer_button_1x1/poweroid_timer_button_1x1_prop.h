#ifndef PROPS_H
#define PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

static struct Properties {

    int8_t DEFAULT_PROPERTY = -1;

    Property FACTORY[3] = {

            //val      minV     maxV       scale
            {1L,       0L,      17L,       1L},
            {0L,       0L,      59L,       1L},
            {0L,       0L,      15L,       1L}
    };

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        FACTORY[0].desc = F("Power time");
        FACTORY[0].measure = 3;

        FACTORY[1].desc = F("Power time");
        FACTORY[1].measure = 4;

        FACTORY[2].desc = F("Power ends");
        FACTORY[2].measure = 4;
    }

} PROPS;

#endif
