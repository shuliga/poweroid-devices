#ifndef FAN_PROPS_H
#define FAN_PROPS_H

#define PWR21

#include "properties.h"
#include "commons.h"
#include "timings.h"

typedef struct Timings{
    TimingState countdown_power, delay_power, light1_standby, light2_standby, humidity_timeout;
};

enum State{
    OFF, AL, AD, POWER, POWER_SBY
};

static State states[1] = {OFF};

static struct Properties{

     Property FACTORY[6] = {

            //desc  value   minV    maxV        scale
            {NULL, 600000L, 60000L, 36000000L, 60000L},
            {NULL, 5000L,   1000,   3600000L,  1000},
            {NULL, 60000L,  1000,   3600000L,  1000},
            {NULL, 60000L,  1000,   3600000L,  1000},
            {NULL, 1800000L, 600000L, 36000000L, 60000L},
            {NULL, 60L,   50L,   99L,  1L}

    };

     long RUNTIME[ARRAY_SIZE(FACTORY)];

// Declaring alternate array Property*[2][4] that represents 2 sets (per each state) of 4 properties
// This makes possible state - related iterations over properties
//    long (*_RUNTIME)[4] = (long (*)[4]) &RUNTIME;

    int props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("POWER-1 run time (min)");
        FACTORY[1].desc = F("POWER-1 delay (sec)");
        FACTORY[2].desc = F("LIGHT-1 stand-by (sec)");
        FACTORY[3].desc = F("LIGHT-2 stand-by (sec)");
        FACTORY[4].desc = F("Humidity timeout (min)");
        FACTORY[5].desc = F("Humidity threshold (%)");
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            RUNTIME[i] = FACTORY[i].val;
        }
    }

} FAN_PROPS;

static char* printState(State state, int8_t i){
    char* ch = new char;
    char* result = new char;
    switch (state){
        case OFF: ch = "OFF";
            break;
        case AL: ch = "AL";
            break;
        case AD: ch = "AD";
            break;
        case POWER: ch = "POWER";
            break;
        case POWER_SBY: ch = "POWER-SBY";
            break;
    }
    sprintf(result, "State[%i]: %s", i, ch);
    return result;
}

#endif
