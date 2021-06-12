#ifndef PROPS_H
#define PROPS_H

#include <properties.h>
#include <commons.h>
#include <timings.h>

static struct Properties {

    int8_t DEFAULT_PROPERTY = -1;

    static const uint8_t props_size = 1;

    Property FACTORY[props_size] = {
            //val      minV      maxV      scale
            {12L,      0L,       24L,      1L}
    };


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

#endif PROPS_H