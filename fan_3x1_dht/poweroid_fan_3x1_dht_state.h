#ifndef FAN_STATE_H
#define FAN_STATE_H

#include "commons.h"
#include "timings.h"

typedef struct Timings {
    TimingState debounce_delay, countdown_power, delay_power, light1_standby, humidity_delay, humidity_runtime, temperature_delay;
};

char *STATE_BUFF = {"DISARM\0OFF\0AL\0AH\0POWER\0POWER-SBY\0"}; // OFFSETS:0,7,11,14,17,23
char *STATE_FORMAT_BUFF = {"[%i] StateLight: %s\0[%i] StateHumid: %s\0[%i] StateTemp: %s"}; // OFFSETS:0, 20, 40

enum StateLight {
    SL_DISARM = 0, SL_OFF = 7, AL = 11, SL_POWER = 17, SL_POWER_SBY = 23
} state_light = SL_OFF, prev_state_light = SL_DISARM;

enum StateHumid {
    SH_DISARM = 0, SH_OFF = 7, AH = 14, SH_POWER = 17, SH_POWER_SBY = 23
} state_humid = SH_OFF, prev_state_humid = SH_DISARM;

enum StateTemp {
    ST_DISARM = 0, ST_OFF = 7, ST_POWER = 17
} state_temp = ST_OFF, prev_state_temp = ST_DISARM;

static uint8_t state_count = 3;
static char CHAR_BUFF_32[32];

static char* printState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_light;break;}
        case 1: {offset = state_humid;break;}
        case 2: {offset = state_temp;break;}
    }
    sprintf(CHAR_BUFF_32, STATE_FORMAT_BUFF + i * 20, i, STATE_BUFF + offset);
    return CHAR_BUFF_32;
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

static void printChangedState(bool prev_state, bool state, uint8_t id){
    if (prev_state != state) {
        printState(id);
    }
}

#endif
