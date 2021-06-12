#include <commons.h>
#include <context.h>
#include <timings.h>
#include <states.h>

#define S_POWER 0
#define S_BASIN 1
#define S_TIMER 2
#define S_INFO 3

enum StatePower {
    SP_DISARM, SP_OFF, SP_AUTO, SP_FILL_IN, SP_MAN_ON
};

enum StateBasin {
    SB_DISARM, SB_INTAKE, SB_FULL, SB_LOW_WATER, SB_SENSOR_FAILED
};

enum StateInfo {
    SI_DISARM, SI_WARNING, SI_ALARM
};

enum StateTimer {
    ST_DISARM, ST_SHUT_DOWN, ST_ENGAGE
};

const char *STATE_NAME_BUFF[] = {"Power", "Basin", "Info", "Timer"};
const char *STATE_POWER[] = {"DISARM", "OFF", "AUTO", "FILL-IN", "MAN. ON"};
const char *STATE_BASIN[] = {"DISARM", "INTAKE", "FULL", "LOW WATR", "US FAIL"};
const char *STATE_INFO[] = {"DISARM","WARN.", "ALARM"};
const char *STATE_TIMER[] = {"DISARM", "SHTDWN",  "ENGAGE"};

StateHolder<StatePower> stateHolderPower(SP_DISARM, SP_AUTO, SP_DISARM, "Power", STATE_POWER);
StateHolder<StateBasin> stateHolderBasin(SB_DISARM, SB_FULL, SB_DISARM, "Basin", STATE_BASIN);
StateHolder<StateTimer> stateHolderTimer(ST_DISARM, ST_SHUT_DOWN, ST_DISARM, "Timer", STATE_TIMER);
StateHolder<StateInfo> stateHolderInfo(SI_DISARM, SI_WARNING, SI_DISARM, "Info", STATE_INFO);

uint8_t const state_count = 4;
StateHolderBase* run_states[state_count] = {(StateHolderBase *) &stateHolderPower, (StateHolderBase *) &stateHolderBasin, (StateHolderBase *) &stateHolderTimer, (StateHolderBase *) &stateHolderInfo};
