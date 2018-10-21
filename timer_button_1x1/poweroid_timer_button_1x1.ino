
#define ID "PWR-TMB-11"

#include "global.h"
#include "Poweroid10.h"
#include "poweroid_timer_button_1x1_state.h"
#include "poweroid_timer_button_1x1_prop.h"
#include "DS1307.h"

Timings timings = {0, 0, 0};
TimingState FLASH(750L);
TimingState FLASH_SBY(250L);

MultiClick btn = MultiClick(IN2_PIN);

const char *printToGo();

Context CTX = Context(SIGNATURE, FULL_VERSION, PROPS.FACTORY, PROPS.props_size, ID,
                      PROPS.DEFAULT_PROPERTY, &printToGo);

Commands CMD(CTX);
Bt BT(CTX.id);

#if !defined(NO_CONTROLLER)
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

void apply_timings() {
    timings.countdown_full.interval = (unsigned long) PROPS.FACTORY[0].runtime * 3600000L +
                                      (unsigned long) PROPS.FACTORY[1].runtime * 60000L;
    timings.countdown_power_end.interval =
            (unsigned long) PROPS.FACTORY[2].runtime * 60000L > timings.countdown_full.interval
            ? timings.countdown_full.interval : PROPS.FACTORY[2].runtime * 60000L;
    timings.countdown_power.interval = timings.countdown_full.interval - timings.countdown_power_end.interval;
}


const char *printToGo() {
    bool countDown = state_power != SP_OFF && state_power != SP_DISARM;
    long totalToGo = countDown ? timings.countdown_full.millsToGo(timings.countdown_full.getCurrent()) / 1000 : 0;
    uint8_t hrsToGo = totalToGo / 3600;
    uint8_t minToGo = (totalToGo - hrsToGo * 3600) / 60;
    uint8_t secToGo = (totalToGo - hrsToGo * 3600 - minToGo * 60);
    if (countDown){
        sprintf(BUFF, "%02d:%02d:%02d", hrsToGo, minToGo, secToGo);
    } else {
        sprintf(BUFF, "%02d:%02d:%02d", RTC.get(2, true), RTC.get(1, false), RTC.get(0, false));
    };
    return BUFF;
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
                timings.countdown_full.reset();
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
                state_power = SP_POWER_END;
                break;
            }
            break;
        }
        case SP_POWER_SBY: {
            timings.countdown_power_end.countdown(true, true, false);
            timings.countdown_power.countdown(true, true, false);
            timings.countdown_full.countdown(true, true, false);
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
            if (prev_state_power == SP_POWER) {
                timings.countdown_power_end.reset();
            }
            prev_state_power = SP_POWER_END;
            if (event == HOLD) {
                state_power = SP_OFF;
                break;
            }
            if (event == CLICK) {
                state_power = SP_POWER_SBY;
                break;
            }
            if (!timings.countdown_power_end.countdown(true, false, false)) {
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
    CMD.printChangedState(prev_state_power, state_power, 0);
}

void setup() {
    PWR.begin();
}

void loop() {

    apply_timings();

    PWR.run();

    run_state_power(btn.checkButton());

    bool power = (state_power == SP_POWER || state_power == SP_POWER_END);

    PWR.power(0, power);
    PWR.power(1, power);

    if (power) {
        if (state_power == SP_POWER) {
            INDICATORS.set(0, true);
        } else {
            INDICATORS.flash(0, &FLASH, true);
        }

    } else {
        if (state_power == SP_POWER_SBY) {
            INDICATORS.flash(0, &FLASH_SBY, true);
        } else {
            INDICATORS.set(0, false);
        }
    }
}