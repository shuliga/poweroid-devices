
#define ID "PWR-TMB-11"

#include <SoftwareSerial.h>
#include <Wire.h>
#include <../../Poweroid_SDK_10/src/global.h>
#include <../../Poweroid_SDK_10/src/Poweroid10.h>
#include "poweroid_timer_button_1x1_state.h"
#include "poweroid_timer_button_1x1_prop.h"
#include <../../Poweroid_SDK_10/lib/MultiClick/MultiClick.h>

#define IND IND_3

MultiClick btn(IN2_PIN);

Timings timings = {0};
unsigned long SBY_MILLS = 0L;

Context CTX = Context(SIGNATURE, FULL_VERSION, PROPS.FACTORY, PROPS.props_size, ID, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER

Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);

#else

Pwr PWR(CTX, &CMD, NULL, &BT);

#endif

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
    uint16_t totalToGo = static_cast<uint16_t>(countDown ? timings.countdown_power.millsToGo() / 1000 : 0);
    uint8_t hrsToGo = static_cast<uint8_t>(totalToGo / 3600);
    uint16_t secToGoM = totalToGo - (hrsToGo * 3600);
    uint8_t minToGo = static_cast<uint8_t>(secToGoM / 60);
    uint8_t secToGo = static_cast<uint8_t>(secToGoM - (minToGo * 60));
    if (countDown){
        sprintf(BANNER.data.text, TIME_FMT , hrsToGo, minToGo, secToGo);
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
