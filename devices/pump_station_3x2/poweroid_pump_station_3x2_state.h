#include <commons.h>
#include <context.h>
#include <timings.h>
#include <states.h>

#define S_POWER 0
#define S_BASIN 1
#define S_PUMP 2
#define S_TIMER 3
#define S_INFO 4

typedef struct Timings {
    TimingState pre_power_timeout;
    TimingState power_fail_delay;
    TimingState countdown_pump_switch;
    TimingState alarm_pump;
    TimingState intake_level_delay;
    TimingState power_timer;
};

enum StatePower {
    SP_DISARM, SP_OFF, SP_PRE_POWER, SP_POWER, SP_DISCHARGE, SP_SUSPEND, SP_FAILED
};

enum StateBasin {
    SB_DISARM, SB_INTAKE, SB_FULL, SB_LOW_WATER, SB_SENSOR_FAILED
};

enum StatePump {
    SPM_PUMP_1, SPM_PUMP_2, SPM_PUMP_1_ONLY, SPM_PUMP_2_ONLY, SPM_PUMP_BOTH, SPM_ALL_FAILED
};

enum StateInfo {
    SI_DISARM, SI_WARNING, SI_ALARM
};

enum StateTimer {
    ST_DISARM, ST_STAND_BY, ST_ENGAGE
};

uint8_t const state_count = 5;

const char *STATE_POWER[] = {"DISARM", "OFF", "PRE-POWER", "POWER", "DISCHRG.", "SUSPEND", "FAIL"};
const char *STATE_BASIN[] = {"DISARM", "INTAKE", "LOW WATER", "FAIL"};
const char *STATE_PUMP[] = {"PUMP 1", "PUMP 2", "P.1 ONLY", "P.2 ONLY", "PUMP BTH", "ALL FAIL"};
const char *STATE_INFO[] = {"DISARM", "WARN.", "ALARM"};
const char *STATE_TIMER[] = {"DISARM", "STAND-BY", "ENGAGE"};

StateHolder<StatePower> stateHolderPower(SP_DISARM, SP_OFF, SP_DISARM, "Power", STATE_POWER);
StateHolder<StateBasin> stateHolderBasin(SB_DISARM, SB_INTAKE, SB_DISARM, "Basin", STATE_BASIN);
StateHolder<StatePump> stateHolderPump(SPM_PUMP_1, SPM_PUMP_1, SPM_PUMP_1_ONLY, "Pump", STATE_PUMP);
StateHolder<StateTimer> stateHolderTimer(ST_DISARM, ST_STAND_BY, ST_DISARM, "Timer", STATE_TIMER);
StateHolder<StateInfo> stateHolderInfo(SI_DISARM, SI_WARNING, SI_DISARM, "Info", STATE_INFO);

StateHolderBase* run_states[state_count] = {(StateHolderBase *) &stateHolderPower, (StateHolderBase *) &stateHolderBasin, (StateHolderBase *) &stateHolderPump, (StateHolderBase *) &stateHolderTimer, (StateHolderBase *) &stateHolderInfo};

