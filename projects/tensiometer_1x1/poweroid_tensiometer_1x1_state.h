#ifndef TENS_STATE_H
#define TENS_STATE_H

#include <../Poweroid_SDK_10/src/commons.h>
#include <../Poweroid_SDK_10/src/context.h>
#include <../Poweroid_SDK_10/src/timings.h>

typedef struct Timings {
    TimingState fill_time;
    TimingState alarm_time;
};

enum StatePower {
    SV_DISARM = 0, SV_READY = 7, SV_OPEN = 13, SP_OPEN_ALARM = 18, SP_ALARM_SHUT = 29
} state_power = SV_READY, prev_state_power = SV_DISARM;

uint8_t state_count = 1;

bool changedState[1] = {false};

const char *STATE_NAME_BUFF[] = {"Valve"};
const char *STATE_BUFF = {"DISARM\0READY\0OPEN\0OPEN-ALARM\0ALARM-SHUT\0",}; // OFFSETS:0,7,13,18,29

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_power;break;}
    }
    run_state.idx = i;
    run_state.name = (char *) STATE_NAME_BUFF[i];
    run_state.state = (char *) STATE_BUFF + offset;
    return &run_state;
}

void gotoStatePower(StatePower newState) {
    prev_state_power = state_power;
    state_power = newState;
    changedState[0] = true;
}

void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case 0: {
            state_power = _disarm ? SV_DISARM : SV_READY;
            prev_state_power = state_power;
            break;
        }
    }
}
#endif
