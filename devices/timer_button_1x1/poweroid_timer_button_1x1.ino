
#define ID "PWR-TMB-11"

#include <global.h>
#include <Poweroid10.h>
#include "poweroid_timer_button_1x1_state.h"
#include "poweroid_timer_button_1x1_prop.h"
#include <MultiClick/MultiClick.h>
#include <timings.h>

#define IND IND_3

MultiClick btn(IN2_PIN);

Timings timings = {0};
unsigned long SBY_MILLS = 0L;

Context CTX = Context(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);

Bt BT(CTX.id);

#ifndef NO_CONTROLLER

Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);

#else

Pwr PWR(CTX, &CMD, NULL, &BT);

#endif

TimeSplit toGo;

McEvent event;
const char * TIME_FMT = "%02d:%02d:%02d";

void applyTimings() {
    timings.countdown_power.interval = (unsigned long) PROPS.FACTORY[0].runtime * 3600000L +
                                      (unsigned long) PROPS.FACTORY[1].runtime * 60000L;
    SBY_MILLS = static_cast<unsigned long>(PROPS.FACTORY[2].runtime * 60000L);
}

void processSensors() {
    event = btn.checkButton();
}

void fillOutput() {
    BANNER.mode=0;
    bool countDown = stateHolderPower.state != SP_OFF && !stateHolderPower.isDisarmed();
    splitTime(countDown ? timings.countdown_power.millsToGo() : 0, toGo);
    if (countDown){
        sprintf(BANNER.data.text, TIME_FMT , toGo.hrs, toGo.mins, toGo.sec);
    } else {
#ifdef CONTROLLER_ONLY
        strcpy(BANNER.data.text, NO_INFO_STR);
#elif defined DATETIME_H
        DATETIME.getTimeString(BANNER.data.text);
#else
        strcpy(BANNER.data.text, NO_INFO_STR);
#endif
    };
}

void run_state_power(McEvent event) {
    switch (stateHolderPower.state) {
        case SP_OFF: {
            stateHolderPower.firstEntry();
            if (event == HOLD) {
                stateHolderPower.gotoState(SP_POWER);
            }
            break;
        }
        case SP_POWER: {
            if (*stateHolderPower.firstEntry() == SP_OFF) {
                timings.countdown_power.reset();
            }

            if (event == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            if (event == CLICK) {
                stateHolderPower.gotoState(SP_POWER_SBY);
                break;
            }
            if (!timings.countdown_power.countdown(true, false, false)) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            if (timings.countdown_power.millsToGo() < SBY_MILLS) {
                stateHolderPower.gotoState(SP_POWER_END);
                break;
            }
            break;
        }
        case SP_POWER_SBY: {
            stateHolderPower.firstEntry();
            timings.countdown_power.countdown(true, true, false);
            if (event == CLICK) {
                stateHolderPower.gotoState(stateHolderPower.prev_stored_state);
                break;
            }
            if (event == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            break;
        }
        case SP_POWER_END: {
            stateHolderPower.firstEntry();
            if (event == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            if (event == CLICK) {
                stateHolderPower.gotoState(SP_POWER_SBY);
                break;
            }
            if (!timings.countdown_power.countdown(true, false, false)) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            break;
        }
        case SP_DISARM: {
            stateHolderPower.firstEntry();
            break;
        }
    }
}

void runPowerStates() {
    run_state_power(event);
}

void setup() {
    PWR.begin();
}

void loop() {

    PWR.run();

    bool power = (stateHolderPower.state == SP_POWER || stateHolderPower.state == SP_POWER_END);

    PWR.power(REL_A, power);
    PWR.power(REL_B, power);

    if (power) {
        if (stateHolderPower.state == SP_POWER) {
            INDICATORS.set(IND, true);
        } else {
            INDICATORS.flash(IND, !flash_accent(timerCounter_1Hz), true);
        }

    } else {
        if (stateHolderPower.state == SP_POWER_SBY) {
            INDICATORS.flash(IND, !flash_accent(timerCounter_4Hz), true);
        } else {
            INDICATORS.set(IND, false);
        }
    }
}
