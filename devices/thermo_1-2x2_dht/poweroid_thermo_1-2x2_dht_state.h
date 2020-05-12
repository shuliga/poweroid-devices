#ifndef THERMO_1-2x2-DHT_STATE_H
#define THERMO_1-2x2-DHT_STATE_H

#include <commons.h>
#include <context.h>
#include <timings.h>

typedef struct Timings {
    TimingState floor_switch_delay, heater_switch_delay;
};

enum StateMode {
    SM_AWAY = 3, SM_ECO = 4, SM_NORMAL = 5
} state_mode = SM_ECO, prev_state_mode = SM_AWAY;

enum StateTempFloor {
    SF_DISARM = 0, SF_OFF = 1, SF_HEAT = 2
} state_temp_floor = SF_OFF, prev_state_temp_floor = SF_DISARM;

enum StateTempHeater {
    SH_DISARM = 0, SH_OFF = 1, SH_HEAT = 2
} state_temp_heater = SH_OFF, prev_state_temp_heater = SH_DISARM;

const uint8_t state_count = 3;
bool changedState[state_count] = {false, false, false};

const char *STATE_NAME_BUFF[] = {"Mode", "Floor t.", "Heater t."};
const char *STATE_BUFF[] = {"DISARM", "OFF", "HEAT", "AWAY", "ECO", "NORM"};

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_mode;break;}
        case 1: {offset = state_temp_floor;break;}
        case 2: {offset = state_temp_heater;break;}
    }
    run_state.idx = i;
    run_state.name = (char *) STATE_NAME_BUFF[i];
    run_state.state = (char *) STATE_BUFF[offset];
    return &run_state;
}


void gotoStateMode(StateMode newState) {
    prev_state_mode = state_mode;
    state_mode = newState;
}

void gotoStateTempFloor(StateTempFloor newState) {
    prev_state_temp_floor = state_temp_floor;
    state_temp_floor = newState;
    changedState[1] = true;
}

void gotoStateTempHeater(StateTempHeater newState) {
    prev_state_temp_heater = state_temp_heater;
    state_temp_heater = newState;
    changedState[2] = true;
}



void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case 0: {
            state_mode = _disarm ? SM_AWAY : SM_ECO;
            break;
        }
        case 1: {
            state_temp_floor = _disarm ? SF_DISARM : SF_OFF;
            break;
        }
        case 2: {
            state_temp_heater = _disarm ? SH_DISARM : SH_OFF;
            break;
        }
    }
}
#endif //THERMO_1-2x2-DHT_STATE_H
