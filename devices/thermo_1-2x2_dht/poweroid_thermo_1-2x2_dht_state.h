#include <commons.h>
#include <context.h>
#include <timings.h>

typedef struct Timings {
    TimingState floor_switch_delay, heater_switch_delay;
};

enum StateMode {
    SM_AWAY = 3, SM_ECO = 4, SM_NORMAL = 5
} state_mode = SM_ECO, prev_state_mode = SM_AWAY;

enum StateTempHeater {
    SH_DISARM = 0, SH_OFF = 1, SH_HEAT = 2
} state_temp_heater = SH_OFF, prev_state_temp_heater = SH_DISARM;

enum StateTempFloor {
    SF_DISARM = 0, SF_OFF = 1, SF_HEAT = 2
}
#ifndef MINI
state_temp_floor = SF_OFF, prev_state_temp_floor = SF_DISARM;
#else
state_temp_floor = SF_DISARM, prev_state_temp_floor = SF_OFF;
#endif

const uint8_t state_count = 3;
bool changedState[state_count] = {false, false, false};

const char *STATE_NAME_BUFF[] = {"Mode", "Heater t.", "Floor t."};
const char *STATE_BUFF[] = {"DISARM", "OFF", "HEAT", "AWAY", "ECO", "NORM"};

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_mode;break;}
        case 1: {offset = state_temp_heater;break;}
        case 2: {offset = state_temp_floor;break;}
        default:;
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

void gotoStateTempHeater(StateTempHeater newState) {
    prev_state_temp_heater = state_temp_heater;
    state_temp_heater = newState;
}

#ifndef MINI
void gotoStateTempFloor(StateTempFloor newState) {
    prev_state_temp_floor = state_temp_floor;
    state_temp_floor = newState;
}
#endif

bool isDisarmedState(uint8_t i){
    switch (i) {
        case 0: {
            return  state_mode ==  SM_AWAY;
        }
        case 1: {
            return  state_temp_heater ==  SH_DISARM;
        }
#ifndef MINI
        case 2: {
            return  state_temp_floor ==  SF_DISARM;
        }
#endif
        default:
            return false;
    }
}

void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case 0: {
            gotoStateMode(_disarm ? SM_AWAY : SM_ECO);
            break;
        }
        case 1: {
            gotoStateTempHeater(_disarm ? SH_DISARM : SH_OFF);
            break;
        }
#ifndef MINI
        case 2: {
            gotoStateTempFloor(_disarm ? SF_DISARM : SF_OFF);
            break;
        }
#endif
        default:;
    }
}
