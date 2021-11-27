#ifndef FAN_STATE_H
#define FAN_STATE_H

#include <commons.h>
#include <context.h>
#include <timings.h>
#include <states.h>

typedef struct Timings {
    TimingState countdown_power;
};

enum StatePower {
    SP_DISARM, SP_OFF, SP_POWER, SP_POWER_SBY, SP_POWER_END
};

const uint8_t state_count = 1;

const char *STATE_POWER[] = {"DISARM", "OFF", "POWER", "POWER-SBY", "POWER-END"};

StateHolder<StatePower> stateHolderPower(SP_DISARM, SP_OFF, SP_DISARM, "Power", STATE_POWER);

StateHolderBase* run_states[state_count] = {(StateHolderBase *) &stateHolderPower};

#endif
