#ifndef FAN_PROPS_H
#define FAN_PROPS_H

#include "properties.h"
#include "commons.h"
#include "timings.h"

#define GET_PROP_NORM(i) FAN_PROPS.RUNTIME[(i)] / FAN_PROPS.FACTORY[(i)].scale
#define TOCBUFF(x) strcpy(CHAR_BUFF_12,x)

typedef struct Timings {
    TimingState debounce_delay, countdown_power, delay_power, light1_standby, humidity_delay, humidity_runtime, temperature_delay;
};

enum StateLight {
    SL_DISARM, SL_OFF, AL, SL_POWER, SL_POWER_SBY
};

enum StateHumid {
    SH_DISARM, SH_OFF, AH, SH_POWER, SH_POWER_SBY
};

enum StateTemp {
    ST_DISARM, ST_OFF, ST_POWER
};

static StateLight state_light = SL_OFF;
static StateHumid state_humid = SH_OFF;
static StateTemp state_temp = ST_OFF;

static uint8_t state_count = 3;
static char CHAR_BUFF_12[12];
static char CHAR_BUFF_24[24];

Bt *BT;

static struct Properties {

    Property FACTORY[8] = {

            //desc  value   minV    maxV        scale
            {NULL, 600000L,  60000L,  36000000L, 60000L},
            {NULL, 5000L,    1000L,   3600000L,  1000L},
            {NULL, 60000L,   1000L,   3600000L,  1000L},
            {NULL, 70L,      50L,     99L,       1L},
            {NULL, 1800000L, 600000L, 36000000L, 60000L},
            {NULL, 300000L,  600000L, 36000000L, 60000L},
            {NULL, 26L,      6L,      35L,       1L},
            {NULL, 300000L,  60000L,  3600000L,  60000L}

    };

    long RUNTIME[ARRAY_SIZE(FACTORY)];

// Declaring alternate array Property*[2][4] that represents 2 sets (per each state) of 4 properties
// This makes possible state - related iterations over properties
//    long (*_RUNTIME)[4] = (long (*)[4]) &RUNTIME;

    uint8_t props_size = ARRAY_SIZE(FACTORY);

// Property strings should be stored in FlashRAM
// to save program RAM space
// Using F() macros here
    Properties() {
        factoryReset();
        FACTORY[0].desc = F("Fan run time (min)");
        FACTORY[1].desc = F("Fan delay (sec)");
        FACTORY[2].desc = F("Light stand-by (sec)");
        FACTORY[3].desc = F("Humidity threshold (%)");
        FACTORY[4].desc = F("Humidity fan run time (min)");
        FACTORY[5].desc = F("Humidity delay (min)");
        FACTORY[6].desc = F("Temperature threshold (~C)");
        FACTORY[7].desc = F("Temperature delay (min)");
    }

    void factoryReset() {
        for (int i = 0; i < ARRAY_SIZE(FACTORY); i++) {
            RUNTIME[i] = FACTORY[i].val;
        }
    }

} FAN_PROPS;

static void printState(uint8_t i) {
    switch (i) {
        case 0: {
            switch (state_light) {
                case SL_DISARM:
                    TOCBUFF("DISARM");
                    break;
                case SL_OFF:
                    TOCBUFF("OFF");
                    break;
                case AL:
                    TOCBUFF("AL");
                    break;
                case SL_POWER:
                    TOCBUFF("POWER");
                    break;
                case SL_POWER_SBY:
                    TOCBUFF("POWER-SBY");
                    break;
                default:
                    return;
            }
            sprintf(CHAR_BUFF_24, "[%i] StateLight: %s", i, CHAR_BUFF_12);
            break;
        }
        case 1: {
            switch (state_humid) {
                case SH_DISARM:
                    TOCBUFF("DISARM");
                    break;
                case SH_OFF:
                    TOCBUFF("OFF");
                    break;
                case AH:
                    TOCBUFF("AH");
                    break;
                case SH_POWER:
                    TOCBUFF("POWER");
                    break;
                case SH_POWER_SBY:
                    TOCBUFF("POWER-SBY");
                    break;
                default:
                    return;
            }
            sprintf(CHAR_BUFF_24, "[%i] StateHumid: %s", i, CHAR_BUFF_12);
            break;
        }
        case 2: {
            switch (state_temp) {
                case ST_DISARM:
                    TOCBUFF("DISARM");
                    break;
                case ST_OFF:
                    TOCBUFF("OFF");
                    break;
                case ST_POWER:
                    TOCBUFF("POWER");
                    break;
                default:
                    return;
            }
            sprintf(CHAR_BUFF_24, "[%i] StateTemp: %s", i, CHAR_BUFF_12);
            break;
        }
    }
    Serial.println(CHAR_BUFF_24);
}

static void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case 0: {
            state_light = _disarm ? SL_DISARM : SL_OFF;
            break;
        }
        case 1: {
            state_humid = _disarm ? SH_DISARM : SH_OFF;
            break;
        }
        case 2: {
            state_temp = _disarm ? ST_DISARM : ST_OFF;
            break;
        }
    }
}

#endif
