
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
    bool countDown = state_power != SP_OFF && state_power != SP_DISARM;
    splitTime(countDown ? timings.countdown_power.millsToGo() : 0, toGo);
    if (countDown){
        sprintf(BANNER.multiplexed.text, TIME_FMT , toGo.hrs, toGo.mins, toGo.sec);
    } else {
#ifdef CONTROLLER_ONLY
        strcpy(BANNER.data.text, NO_INFO_STR);
#elif defined DATETIME_H
        DATETIME.getTimeString(BANNER.data.text);
#else
        strcpy(BANNER.multiplexed.text, NO_INFO_STR);
#endif
    };
}

void run_state_power(McEvent event) {
    switch (state_power) {
        case SP_OFF: {
            prev_state_power = SP_OFF;
            if (event == HOLD) {
                state_power = SP_POWER;
            }
            break;
        }
        case SP_POWER: {
            if (prev_state_power == SP_OFF) {
                timings.countdown_power.reset();
            }
            prev_state_power = SP_POWER;
            if (event == HOLD) {
                state_power = SP_OFF;
                break;
            }
            if (event == CLICK) {
                state_power = SP_POWER_SBY;
                break;
            }
            if (!timings.countdown_power.countdown(true, false, false)) {
                state_power = SP_OFF;
                break;
            }
            if (timings.countdown_power.millsToGo() < SBY_MILLS) {
                state_power = SP_POWER_END;
                break;
            }
            break;
        }
        case SP_POWER_SBY: {
            timings.countdown_power.countdown(true, true, false);
            if (event == CLICK) {
                state_power = prev_state_power;
                break;
            }
            if (event == HOLD) {
                state_power = SP_OFF;
                break;
            }
            break;
        }
        case SP_POWER_END: {
            prev_state_power = SP_POWER_END;
            if (event == HOLD) {
                state_power = SP_OFF;
                break;
            }
            if (event == CLICK) {
                state_power = SP_POWER_SBY;
                break;
            }
            if (!timings.countdown_power.countdown(true, false, false)) {
                state_power = SP_OFF;
                break;
            }
            break;
        }
        case SP_DISARM: {
            prev_state_power = SP_DISARM;
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

    bool power = (state_power == SP_POWER || state_power == SP_POWER_END);

    PWR.power(REL_A, power);
    PWR.power(REL_B, power);

    if (power) {
        if (state_power == SP_POWER) {
            INDICATORS.set(IND, true);
        } else {
            INDICATORS.flash(IND, !flash_accent(timerCounter_1Hz), true);
        }

    } else {
        if (state_power == SP_POWER_SBY) {
            INDICATORS.flash(IND, !flash_accent(timerCounter_4Hz), true);
        } else {
            INDICATORS.set(IND, false);
        }
    }
}
