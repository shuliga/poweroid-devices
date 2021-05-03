
#define ID "THERMO_1-2x2-DHT"

#include <Poweroid10.h>
#include <MultiClick/MultiClick.h>
#include "poweroid_thermo_1-2x2_dht_state.h"
#include "poweroid_thermo_1-2x2_dht_prop.h"

#define TEMP_FAIL INT8_MIN

MultiClick btn_override(IN2_PIN);

typedef struct Timings {
    TimingState floor_switch_delay, heater_switch_delay;
};

Timings timings = {0, 0};

Context CTX = Context(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

McEvent event[2];

int8_t current_temp;
int8_t floor_temp;
int8_t target_heater_temp;
bool inverse;

void applyTimings() {
    timings.floor_switch_delay.interval = (unsigned long) PROPS.FACTORY[5].runtime;
    timings.heater_switch_delay.interval = (unsigned long) PROPS.FACTORY[6].runtime;
}

void processSensors() {
    if (PWR.SENS->isDhtInstalled()) {
        PWR.SENS->setTempCorrection(PROPS.FACTORY[7].runtime);
//        PWR.SENS->setHumidCorrection(PROPS.FACTORY[8].runtime);
        float temp = PWR.SENS->getTemperature();
        current_temp = (!isnan(temp) && temp > -20 && temp < 40) ? PWR.SENS->getInt(temp) : TEMP_FAIL;
    } else {
        current_temp = TEMP_FAIL;
    }

// On MINI boards (with 2 sensor sockets) ECO/NORMAL switch is not available
#ifndef MINI
    McEvent currEvent = CTX.SENS.isSensorOn(SEN_3) ? PRESSED : RELEASED;
    inverse = event[0] != currEvent ? false : inverse;
    event[0] = currEvent;
#else
    event[0] = RELEASED;
#endif

    event[1] = btn_override.checkButton();
    inverse = event[1] == CLICK == !inverse;
}

bool shouldUpdate() {
    return CTX.propsUpdated || stateHolderMode.wasChanged() || stateHolderTempHeater.wasChanged() || current_temp == TEMP_FAIL;
}

void fillOutput() {
    BANNER.mode = 0;
    strcpy(BANNER.data.text, PWR.SENS->printDht());
}


void run_state_mode(const McEvent _event[]) {
    switch (stateHolderMode.state) {
        case SM_AWAY: {
            if (stateHolderMode.firstState(SM_AWAY)) {
                CTX.PERS.storeState(0, true);
            }

            floor_temp = PROPS.FACTORY[4].runtime;
            target_heater_temp = PROPS.FACTORY[4].runtime;

            if (_event[1] == HOLD) {
                CTX.PERS.storeState(0, false);
                stateHolderMode.gotoState(SM_ECO);
            }
            break;
        }
        case SM_ECO: {
            stateHolderMode.firstState(SM_ECO);

            floor_temp = PROPS.FACTORY[2].runtime;
            target_heater_temp = PROPS.FACTORY[3].runtime;

            if (inverse ? _event[0] == PRESSED : _event[0] == RELEASED) {
                stateHolderMode.gotoState(SM_NORMAL);
            }
            if (_event[1] == HOLD) {
                stateHolderMode.gotoState(SM_AWAY);
            }
            break;
        }
        case SM_NORMAL: {
            stateHolderMode.firstState(SM_NORMAL);

            floor_temp = PROPS.FACTORY[0].runtime;
            target_heater_temp = PROPS.FACTORY[1].runtime;

            if (inverse ? _event[0] == RELEASED : _event[0] == PRESSED) {
                stateHolderMode.gotoState(SM_ECO);
            }
            if (_event[1] == HOLD) {
                stateHolderMode.gotoState(SM_AWAY);
            }
            break;
        }
    }
}

void run_state_temp_heater() {
    bool doHeat = current_temp != TEMP_FAIL && current_temp < target_heater_temp;
    switch (stateHolderTempHeater.state) {
        case SH_DISARM:{
            firstStateTempHeater(SH_DISARM);
            break;
        }
        case SH_OFF: {
            stateHolderTempHeater.firstState(SH_OFF);
            if (timings.heater_switch_delay.isTimeAfter(doHeat) || (doHeat && (shouldUpdate() || stateHolderTempHeater.prev_state == SH_DISARM))) {
                stateHolderTempHeater.gotoState(SH_HEAT);
            }
            break;
        }
        case SH_HEAT: {
            stateHolderTempHeater.firstState(SH_HEAT);
            if (timings.heater_switch_delay.isTimeAfter(!doHeat) || (shouldUpdate() && !doHeat)) {
                stateHolderTempHeater.gotoState(SH_OFF);
            }
            break;
        }
    }
}

#ifndef MINI
void run_state_temp_floor() {
    bool doHeat = current_temp != TEMP_FAIL && current_temp < floor_temp;
    switch (stateHolderTempFloor.state) {
    switch (state_temp_floor) {
        case SF_DISARM:{
            firstStateTempFloor(SF_DISARM);
            break;
        }
        case SF_OFF: {
            firstStateTempFloor(SF_OFF);
            if (timings.floor_switch_delay.isTimeAfter(doHeat) || (doHeat && (shouldUpdate() || prev_state_temp_floor == SF_DISARM))) {
                gotoStateTempFloor(SF_HEAT);
            }
            break;
        }
        case SF_HEAT: {
            firstStateTempFloor(SF_HEAT);
            if (timings.floor_switch_delay.isTimeAfter(!doHeat) || (shouldUpdate() && !doHeat)) {
                gotoStateTempFloor(SF_OFF);
            }
            break;
        }
    }
}
#endif

void runPowerStates() {
    run_state_mode(event);
    run_state_temp_heater();
#ifndef MINI
    run_state_temp_floor();
#endif
}

void setup() {
    PWR.begin();
}

void loop() {

    PWR.run();
    PWR.power(REL_A, stateHolderTempHeater.state == SH_HEAT);
#ifndef MINI
    PWR.power(REL_B, stateHolderTempFloor.state == SF_HEAT);
#endif

    if (current_temp == TEMP_FAIL) {
        INDICATORS.flash(IND_1, flash_(timerCounter_4Hz), true);
    } else if (stateHolderMode.state == SM_ECO || stateHolderMode.state == SM_NORMAL) {
        INDICATORS.set(IND_1, stateHolderMode.state == SM_ECO);
    } else {
        INDICATORS.flash(IND_1, !flash_accent(timerCounter_1Hz), stateHolderMode.state == SM_AWAY);
    }

}
