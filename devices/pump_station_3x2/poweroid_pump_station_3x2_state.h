#ifndef FAN_STATE_H
#define FAN_STATE_H

#include <commons.h>
#include <context.h>
#include <timings.h>

typedef struct Timings {
    TimingState countdown_pump;
    TimingState alarm_pump;
    TimingState countdown_pre_power;
    TimingState countdown_lost_power;
};

enum StatePower {
    SP_DISARM = 0, SP_OFF = 1, SP_PRE_POWER = 2, SP_POWER = 3, SP_DISCHARGE = 4
} state_power = SP_OFF, prev_state_power = SP_DISARM;

enum StateBasin {
    SB_DISARM = 0, SB_INTAKE = 7, SB_LOW_WATER = 11, SB_SENSOR_FAILED = 8
} state_basin = SB_INTAKE, prev_state_basin = SB_DISARM;

enum StatePump {
    SPM_PUMP_1 = 48, SPM_PUMP_2 = 55, SPM_PUMP_1_ONLY = 62, SPM_PUMP_2_ONLY = 74, SPM_PUMP_BOTH = 86, SPM_ALL_FAILED = 96
} state_pump = SPM_PUMP_1, prev_state_pump = SPM_PUMP_2;

enum StateInfo {
    SI_DISARM = 0, SI_WARNING = 112, SI_ALARM
} state_info = SI_DISARM, prev_state_info = SI_WARNING;

uint8_t const state_count = 4;

bool changedState[state_count] = {false, false, false, false};

const char *STATE_NAME_BUFF[] = {"Power", "Basing", "Pump", "Info"};
const char *STATE_BUFF[] = {"DISARM", "OFF", "PRE-POWER", "POWER", "DISCHARGE", "INTAKE", "LOW WATER", "FAIL", "PUMP 1", "PUMP 2", "PUMP 1 ONLY", "PUMP 2 ONLY", "PUMP BOTH", "ALL FAILED", "ALARM"};

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_power;break;}
        case 1: {offset = state_pump;break;}
        case 2: {offset = state_info;break;}
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

void gotoStatePump(StatePump newState) {
    prev_state_pump = state_pump;
    state_pump = newState;
    changedState[1] = true;
}

void gotoStateInfo(StateInfo newState) {
    prev_state_info = state_info;
    state_info = newState;
    changedState[2] = true;
}


void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case 0: {
            state_power = _disarm ? SP_DISARM : SP_OFF;
            break;
        }
    }
}
#endif
