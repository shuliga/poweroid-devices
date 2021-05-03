#include <commons.h>
#include <context.h>
#include <timings.h>
#include <states.h>

enum StateMode {
    SM_AWAY, SM_ECO, SM_NORMAL
};

enum StateTempHeater {
    SH_DISARM, SH_OFF, SH_HEAT
};

#ifndef MINI
enum StateTempFloor {
    SF_DISARM, SF_OFF, SF_HEAT
};
#endif

const char *STATE_MODE[] = {"AWAY", "ECO", "NORM"};
const char *STATE_HEATER[] = {"DISARM", "OFF", "HEAT"};

StateHolder<StateMode> stateHolderMode(SM_AWAY, SM_ECO, SM_AWAY, "Mode", STATE_MODE);
StateHolder<StateTempHeater> stateHolderTempHeater(SH_DISARM, SH_OFF, SH_DISARM, "Heater t.", STATE_HEATER);
#ifndef MINI
StateHolder<StateTempFloor> stateHolderTempFloor(SF_DISARM, SF_OFF, SF_DISARM, "Floor t.", STATE_HEATER);

const uint8_t state_count = 3;
StateHolderBase* run_states[state_count] = {(StateHolderBase *) &stateHolderMode, (StateHolderBase *) &stateHolderTempHeater, (StateHolderBase *) &stateHolderTempFloor};
#else
const uint8_t state_count = 2;
StateHolderBase* run_states[state_count] = {(StateHolderBase *) &stateHolderMode, (StateHolderBase *) &stateHolderTempHeater};
#endif