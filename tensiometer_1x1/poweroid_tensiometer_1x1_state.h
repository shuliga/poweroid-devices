#ifndef FAN_STATE_H
#define FAN_STATE_H

#include <../Poweroid_SDK_10/src/commons.h>
#include <../Poweroid_SDK_10/src/context.h>
#include <../Poweroid_SDK_10/src/timings.h>

typedef struct Timings {
    TimingState countdown_power;
};

enum StatePower {
    SP_DISARM = 0, SP_OFF = 7, SP_POWER = 11, SP_PRE_POWER = 17, SP_LOST_POWER = 27, SP_LOW_LEVEL = 38
} state_power = SP_OFF, prev_state_power = SP_DISARM;

enum StatePump {
    SPM_PUMP_1 = 48, SPM_PUMP_2 = 55, SPM_PUMP_BOTH = 62
} state_pump = SPM_PUMP_1, prev_state_pump = SPM_PUMP_2;

enum StateInfo {
    SI_DISARM = 0, SI_ALARM = 62
} state_info = SI_DISARM, prev_state_info = SI_ALARM;

uint8_t state_count = 2;

const char *STATE_NAME_BUFF[] = {"Power", "Pump", "Info"};
const char *STATE_BUFF = {"DISARM\0OFF\0POWER\0PRE-POWER\0LOST-POWER\0ALARM\0LOW_LEVEL\0PUMP_1\0PUMP_2\0PUMP_BOTH\0ALARM\0",}; // OFFSETS:0,7,11,17,27,38,48,55,62

RunState run_state;

RunState *getState(uint8_t i) {
    uint8_t offset = 0;
    switch (i) {
        case 0: {offset = state_power;break;}
        case 1: {offset = state_pump;break;}
        case 3: {offset = state_info;break;}
    }
    run_state.idx = i;
    run_state.name = (char *) STATE_NAME_BUFF[i];
    run_state.state = (char *) STATE_BUFF + offset;
    return &run_state;
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
