
#define ID "PWR-FAN-FLR-32-DHT"

#include <SoftwareSerial.h>
#include <Wire.h>
#include <../../Poweroid_SDK_10/src/global.h>
#include <../../Poweroid_SDK_10/src/Poweroid10.h>
#include "poweroid_thermo_1-2x2_dht_state.h"
#include "poweroid_thermo_1-2x2_dht_prop.h"
#include "../../Poweroid_SDK_10/src/boards.h"
#include <../../Poweroid_SDK_10/lib/MultiClick/MultiClick.h>

MultiClick btn_eco(IN2_PIN);
MultiClick btn_override(IN3_PIN);

Timings timings = {0, 0};

Context CTX = Context(SIGNATURE, FULL_VERSION, PROPS.FACTORY, PROPS.props_size, ID, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

McEvent event[2];

int8_t curent_temp;
int8_t floor_temp;
int8_t heater_temp;

void applyTimings() {
    timings.heater_switch_delay.interval = (unsigned long)  PROPS.FACTORY[5].runtime;
    timings.floor_switch_delay.interval = (unsigned long) PROPS.FACTORY[6].runtime;
}

void fillOutput() {
    BANNER.mode = 0;
    strcpy(BANNER.data.text, PWR.SENS->printDht());
}

void run_state_mode(McEvent _event[]) {
    switch (state_mode) {
        case SM_AWAY: {
            if (prev_state_mode != SM_AWAY) {
                floor_temp = PROPS.FACTORY[4].runtime;
                heater_temp = PROPS.FACTORY[4].runtime;
                prev_state_mode = SM_AWAY;
                changedState[0] = true;
            }
            if (_event[1] == HOLD) {
                gotoStateMode(SM_ECO);
            }
            break;
        }
        case SM_ECO: {
            if (prev_state_mode != SM_ECO) {
                floor_temp =  PROPS.FACTORY[1].runtime;
                heater_temp = PROPS.FACTORY[3].runtime;
                prev_state_mode = SM_ECO;
                changedState[0] = true;
            }
            if (_event[1] == CLICK || _event[0] == RELEASED || _event[0] == DOUBLE_CLICK) {
                gotoStateMode(SM_NORMAL);
            }
            if (_event[1] == HOLD) {
                gotoStateMode(SM_AWAY);
            }
            break;
        }
        case SM_NORMAL: {
            if (prev_state_mode != SM_NORMAL) {
                floor_temp = PROPS.FACTORY[0].runtime;
                heater_temp = PROPS.FACTORY[2].runtime;
                prev_state_mode = SM_NORMAL;
                changedState[0] = true;
            }
            if (_event[1] == CLICK || _event[0] ==  PRESSED) {
                gotoStateMode(SM_ECO);
            }
            if (_event[1] == HOLD) {
                gotoStateMode(SM_AWAY);
            }
            break;
        }
    }
    _event[0] = NOTHING;
    _event[1] = NOTHING;
}

void run_state_temp_floor() {
    bool doHeat = curent_temp < floor_temp;
    switch (state_temp_floor) {
        case SF_OFF: {
            if (timings.floor_switch_delay.isTimeAfter(doHeat) || (changedState[0] && doHeat)) {
                gotoStateTempFloor(SF_HEAT);
            }
            break;
        }
        case SF_HEAT: {
            if (timings.floor_switch_delay.isTimeAfter(!doHeat) || (changedState[0] && !doHeat)) {
                gotoStateTempFloor(SF_OFF);
            }
            break;
        }
    }
}

void run_state_temp_heater() {
    bool doHeat = curent_temp < heater_temp;
    switch (state_temp_heater) {
        case SH_OFF: {
            if (timings.heater_switch_delay.isTimeAfter(doHeat) || (changedState[0] && doHeat)) {
                gotoStateTempHeater(SH_HEAT);
            }
            break;
        }
        case SH_HEAT: {
            if (timings.heater_switch_delay.isTimeAfter(!doHeat) || (changedState[0] && !doHeat)) {
                gotoStateTempHeater(SH_OFF);
            }
            break;
        }
    }
}

void processSensors() {

    if (PWR.SENS->isDhtInstalled()) {
        curent_temp = round(PWR.SENS->getTemperature());
    }

    event[0] = btn_eco.checkButton();
    event[1] = btn_override.checkButton();

}

void runPowerStates() {
    run_state_mode(event);
    run_state_temp_floor();
    run_state_temp_heater();

}

void setup() {
    PWR.begin();
}

void loop() {

    PWR.run();

    PWR.power(REL_A, state_temp_floor == SF_HEAT);
    PWR.power(REL_B, state_temp_heater == SH_HEAT);

    if (state_mode == SM_ECO || state_mode == SM_NORMAL){
        INDICATORS.set(IND_1, state_mode == SM_ECO);
    } else {
        INDICATORS.flash(IND_1, flash_symm(timerCounter_4Hz), state_mode == SM_AWAY);
    }
}
