#ifndef FAN_STATE_H
#define FAN_STATE_H

#include <../../Poweroid_SDK_10/src/commons.h>
#include <../../Poweroid_SDK_10/src/context.h>
#include <../../Poweroid_SDK_10/src/timings.h>

typedef struct Timings {
    TimingState debounce_delay, countdown_power, delay_power, light1_standby, humidity_delay, humidity_runtime, temperature_delay;
};

enum StateLight {
    SL_DISARM = 0, SL_OFF = 7, AL = 11, SL_POWER = 17, SL_POWER_SBY = 23
} state_light = SL_OFF, prev_state_light = SL_DISARM;

enum StateHumid {
    SH_DISARM = 0, SH_OFF = 7, AH = 14, SH_POWER = 17
} state_humid = SH_OFF, prev_state_humid = SH_DISARM;

enum StateTemp {
    ST_DISARM = 0, ST_OFF = 7, ST_POWER = 17
} state_temp = ST_OFF, prev_state_temp = ST_DISARM;

uint8_t state_count = 3;
bool changedState[3] = {false, false, false};

const char *STATE_NAME_BUFF[] = {"Lightning", "Humidity", "Temperature"};
const char *STATE_BUFF = {"DISARM\0OFF\0AL\0AH\0POWER\0PWR-SBY\0"}; // OFFSETS:0,7,11,14,17,23

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_light;break;}
        case 1: {offset = state_humid;break;}
        case 2: {offset = state_temp;break;}
    }
    run_state.idx = i;
    run_state.name = (char *) STATE_NAME_BUFF[i];
    run_state.state = (char *) STATE_BUFF + offset;
    return &run_state;
}


void gotoStateLight(StateLight newState) {
    prev_state_light = state_light;
    state_light = newState;
    changedState[0] = true;
}

void gotoStateHumid(StateHumid newState) {
    prev_state_humid = state_humid;
    state_humid = newState;
    changedState[1] = true;
}

void gotoStateTemp(StateTemp newState) {
    prev_state_temp = state_temp;
    state_temp = newState;
    changedState[2] = true;
}



void disarmState(uint8_t i, bool _disarm) {
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
