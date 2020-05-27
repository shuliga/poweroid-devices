#include <commons.h>
#include <context.h>
#include <timings.h>

#define S_POWER 0
#define S_PUMP 1
#define S_BASIN 2
#define S_INFO 3
#define S_TIMER 4

typedef struct Timings {
    TimingState pre_power_timeout;
    TimingState power_fail_delay;
    TimingState countdown_pump_switch;
    TimingState alarm_pump;
    TimingState intake_level_delay;
};

enum StatePower {
    SP_DISARM = 0, SP_OFF = 1, SP_PRE_POWER = 2, SP_POWER = 3, SP_DISCHARGE = 4, SP_SUSPEND = 5, SP_FAILED = 8
} state_power = SP_OFF, prev_state_power = SP_DISARM;

enum StateBasin {
    SB_DISARM = 0, SB_INTAKE = 6, SB_LOW_WATER = 7, SB_SENSOR_FAILED = 8
} state_basin = SB_INTAKE, prev_state_basin = SB_DISARM;

enum StatePump {
    SPM_PUMP_1 = 9, SPM_PUMP_2 = 10, SPM_PUMP_1_ONLY = 11, SPM_PUMP_2_ONLY = 12, SPM_PUMP_BOTH = 13, SPM_ALL_FAILED = 14
} state_pump = SPM_PUMP_1, prev_state_pump = SPM_PUMP_2;

enum StateInfo {
    SI_DISARM = 0, SI_WARNING = 15, SI_ALARM = 16
} state_info = SI_DISARM, prev_state_info = SI_WARNING;

enum StateTimer {
    ST_DISARM = 0, ST_STAND_BY = 17, ST_ENGAGE = 18
} state_timer = ST_DISARM, prev_state_timer = ST_STAND_BY;

uint8_t const state_count = 5;

bool changedState[state_count] = {false, false, false, false, false};

const char *STATE_NAME_BUFF[] = {"Power", "Pump", "Basin", "Info", "Timer"};
const char *STATE_BUFF[] = {"DISARM", "OFF", "PRE-POWER", "POWER", "DISCHRG.", "SUSPEND", "INTAKE", "LOW WATER", "FAIL", "PUMP 1", "PUMP 2", "P.1 ONLY", "P.2 ONLY", "PUMP BTH", "ALL FAIL", "WARN.", "ALARM", "STAND-BY", "ENGAGE"};

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case S_POWER: {offset = state_power;break;}
        case S_PUMP: {offset = state_pump;break;}
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

void gotoStatePump(StatePump newState) {
    prev_state_pump = state_pump;
    state_pump = newState;
    changedState[S_PUMP] = true;
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
        case S_PUMP: return state_pump == SPM_PUMP_1_ONLY;
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
        case S_PUMP: {
            state_pump = _disarm ? SPM_PUMP_1_ONLY : SPM_PUMP_1;
            break;
        }
        case S_BASIN: {
            state_basin = _disarm ? SB_DISARM : SB_INTAKE;
            break;
        }
        case S_INFO: {
            state_info = _disarm ? SI_DISARM : SI_WARNING;
            break;
        }
        case S_TIMER: {
            state_timer = _disarm ? ST_DISARM : ST_STAND_BY;
            break;
        }
        default:;
    }
}
