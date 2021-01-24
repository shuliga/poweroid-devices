#ifndef TENS_STATE_H
#define TENS_STATE_H

#include <commons.h>
#include <context.h>
#include <timings.h>

typedef struct Timings {
    TimingState fill_time;
    TimingState alarm_time;
};

enum StatePower {
    SP_DISARM = 0, SP_READY = 1, SP_OPEN = 2, SP_OPEN_ALARM = 3, SP_ALARM_SHUT = 4
} state_power = SP_READY, prev_state_power = SP_DISARM;

uint8_t const state_count = 1;

bool changedState[1] = {false};

const char *STATE_NAME_BUFF[] = {"Valve"};
const char *STATE_BUFF[] = {"DISARM", "READY", "OPEN", "OPEN-ALARM", "ALARM-SHUT"};

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_power;break;}
    }
    run_state.idx = i;
    run_state.name = (char *) STATE_NAME_BUFF[i];
    run_state.state = (char *) STATE_BUFF[offset];
    return &run_state;
}

void gotoStatePower(StatePower newState) {
    prev_state_power = state_power;
    state_power = newState;
    changedState[0] = true;
}

bool isDisarmedState(uint8_t i){
    switch (i) {
        case 0: {
            return  state_power ==  SP_DISARM;
        }
        default:
            return false;
    }
}

void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case 0: {
            state_power = _disarm ? SP_DISARM : SP_READY;
            prev_state_power = state_power;
            break;
        }
    }
}
#endif
