#include <commons.h>
#include <context.h>
#include <states.h>
#include <timings.h>

typedef struct Timings {
    TimingState floor_switch_delay, heater_switch_delay;
};

enum StateMode {
    SM_AWAY, SM_ECO, SM_NORMAL
};

enum StateTempHeater {
    SH_DISARM, SH_OFF, SH_HEAT
};

enum StateTempFloor {
    SF_DISARM, SF_OFF, SF_HEAT
}

#ifndef MINI
state_temp_floor = SF_OFF, prev_state_temp_floor = SF_DISARM;
#else
state_temp_floor = SF_DISARM, prev_state_temp_floor = SF_OFF;
#endif

const uint8_t state_count = 3;
bool changedState[state_count] = {false, false, false};

const char *STATE_MODE[] = {"AWAY", "ECO", "NORM"};
const char *STATE_HEATER[] = {"DISARM", "OFF", "HEAT"};

StateHolder<StateMode> stateHolderMode(SM_AWAY, SM_ECO, SM_AWAY, "Mode", STATE_MODE);
StateHolder<StateTempHeater> stateHolderTempHeater(SH_DISARM, SH_OFF, SH_DISARM, "Heater t.", STATE_HEATER);
StateHolder<StateTempFloor> stateHolderTempFloor(SF_DISARM, SF_OFF, SF_DISARM, "Floor t.", STATE_HEATER);

StateHolderBase* sb = &stateHolderMode;

static const StateHolderBase* run_states[state_count] = {&stateHolderMode, &stateHolderTempHeater, &stateHolderTempFloor};

