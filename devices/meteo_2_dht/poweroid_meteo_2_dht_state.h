#ifndef TENS_STATE_H
#define TENS_STATE_H

#include <commons.h>
#include <context.h>
#include <timings.h>
#include <states.h>

enum StateSystem {
    SG_DISARM, SG_OPERATING, SG_ALARM
};

enum StateRain {
    SR_DISARM, SR_DRY, SR_RAINING
};

const char *STATE_BUFF_SYSTEM[] = {"DISARM", "OPERATING", "ALARM"};
const char *STATE_BUFF_RAIN[] = {"DISARM", "DRY", "RAINING"};

StateHolder<StateSystem> stateHolderSystem(SG_DISARM, SG_OPERATING, SG_DISARM, "System", STATE_BUFF_SYSTEM);
StateHolder<StateRain> stateHolderRain(SR_DISARM, SR_DRY, SR_DISARM, "Rain", STATE_BUFF_RAIN);

uint8_t const state_count = 2;
StateHolderBase* run_states[state_count] = {(StateHolderBase *) & stateHolderSystem, (StateHolderBase *) & stateHolderRain};

#endif // TENS_STATE_H
