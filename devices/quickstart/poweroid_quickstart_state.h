#ifndef TENS_STATE_H
#define TENS_STATE_H

#include <commons.h>
#include <context.h>
#include <timings.h>
#include <states.h>

typedef struct Timings {
    TimingState fill_time;
    TimingState alarm_time;
};

enum StatePower {
    SP_DISARM, SP_READY, SP_OPEN, SP_OPEN_ALARM, SP_ALARM_SHUT
};

const char *STATE_BUFF[] = {"DISARM", "READY", "OPEN", "OPEN-ALARM", "ALARM-SHUT"};

StateHolder<StatePower> stateHolderPower(SP_DISARM, SP_READY, SP_DISARM, "Valve", STATE_BUFF);

uint8_t const state_count = 1;
StateHolderBase* run_states[state_count] = {(StateHolderBase *) &stateHolderPower};

#endif // TENS_STATE_H
