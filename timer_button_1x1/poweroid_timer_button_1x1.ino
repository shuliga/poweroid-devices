
#define ID "PWR-FAN-FLR-32-DHT"

#include "global.h"
#include "Poweroid10.h"
#include "poweroid_timer_button_1x1_state.h"
#include "poweroid_timer_button_1x1_prop.h"

Timings timings = {0, 0};
TimingState FLASH(500L);

MultiClick btn = MultiClick(IN3_PIN);

Context CTX = Context(SIGNATURE, FULL_VERSION, PROPS.FACTORY, PROPS.props_size, ID,
                      PROPS.DEFAULT_PROPERTY);

Commands CMD(CTX);
Bt BT(CTX.id);

#if !defined(NO_CONTROLLER)
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

void apply_timings() {
    timings.countdown_power_end.interval = (unsigned long) PROPS.FACTORY[2].runtime * 60000L;
    timings.countdown_power.interval = (unsigned long) PROPS.FACTORY[0].runtime * 3600000L +
                                       (unsigned long) PROPS.FACTORY[1].runtime * 60000L -
                                       timings.countdown_power_end.interval;
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
            if (timings.countdown_power.isTimeAfter(true)) {
                state_power = SP_POWER_END;
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
            if (timings.countdown_power_end.isTimeAfter(true)) {
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
        INDICATORS.set(0, false);
    }
}
