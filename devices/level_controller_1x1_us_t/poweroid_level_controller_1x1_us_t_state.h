#include <commons.h>
#include <context.h>
#include <timings.h>

#define S_POWER 0
#define S_BASIN 1
#define S_INFO 2
#define S_TIMER 3

enum StatePower {
    SP_DISARM = 0,  SP_OFF = 1, SP_AUTO = 2, SP_FILL_IN = 3
} state_power = SP_AUTO, prev_state_power = SP_DISARM;

enum StateBasin {
    SB_DISARM = 0, SB_INTAKE = 4, SB_FULL = 5, SB_LOW_WATER = 6, SB_SENSOR_FAILED = 7
} state_basin = SB_FULL, prev_state_basin = SB_DISARM;

enum StateInfo {
    SI_DISARM = 0, SI_WARNING = 8, SI_ALARM = 9
} state_info = SI_DISARM, prev_state_info = SI_WARNING;

enum StateTimer {
    ST_DISARM = 0, ST_SHUT_DOWN = 10, ST_ENGAGE = 11
} state_timer = ST_DISARM, prev_state_timer = ST_ENGAGE;

uint8_t const state_count = 4;

bool changedState[state_count] = {false, false, false, false};

const char *STATE_NAME_BUFF[] = {"Power", "Basin", "Info", "Timer"};
const char *STATE_BUFF[] = {"DISARM", "OFF", "AUTO", "FILL-IN", "INTAKE", "FULL", "LOW WATR", "US FAIL", "WARN.", "ALARM", "STAND-BY", "SHUTDOWN"};

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case S_POWER: {offset = state_power;break;}
        case S_BASIN: {offset = state_basin;break;}
        case S_INFO: {offset = state_info;break;}
        case S_TIMER: { offset = state_timer;break;}
    }
    run_state.idx = i;
    run_state.name = (char *) STATE_NAME_BUFF[i];
    run_state.state = (char *) STATE_BUFF[offset];
    return &run_state;
}

void gotoStatePower(StatePower newState) {
    prev_state_power = state_power;
    state_power = newState;
    changedState[S_POWER] = true;
}

void gotoStateBasin(StateBasin newState) {
    prev_state_basin = state_basin;
    state_basin = newState;
    changedState[S_BASIN] = true;
}

void gotoStateInfo(StateInfo newState) {
    prev_state_info = state_info;
    state_info = newState;
    changedState[S_INFO] = true;

}

void gotoStateTimer(StateTimer newState) {
    prev_state_timer = state_timer;
    state_timer = newState;
    changedState[S_TIMER] = true;
}

bool  isDisarmedState(uint8_t i) {
    switch (i) {
        case S_POWER: return state_power == SP_DISARM;
        case S_BASIN: return state_basin == SB_DISARM;
        case S_INFO: return state_info == SI_DISARM;
        case S_TIMER: return state_timer == ST_DISARM;
        default: return false;
    }
}

void disarmState(uint8_t i, bool _disarm) {
    switch (i) {
        case S_POWER: {
            state_power = _disarm ? SP_DISARM : SP_OFF;
            break;
        }
        case S_BASIN: {
            state_basin = _disarm ? SB_DISARM : SB_FULL;
            break;
        }
        case S_INFO: {
            state_info = _disarm ? SI_DISARM : SI_WARNING;
            break;
        }
        case S_TIMER: {
            state_timer = _disarm ? ST_DISARM : ST_SHUT_DOWN;
            break;
        }
        default:;
    }
}
