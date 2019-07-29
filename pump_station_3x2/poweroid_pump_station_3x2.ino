
#define ID "PWR-PMS-32"

#include <SoftwareSerial.h>
#include <Wire.h>
#include <../Poweroid_SDK_10/src/global.h>
#include <../Poweroid_SDK_10/src/Poweroid10.h>
#include "poweroid_pump_station_3x2_state.h"
#include "poweroid_pump_station_3x2_prop.h"
#include <../Poweroid_SDK_10/src//ultrasonic.h>
#include <../Poweroid_SDK_10/lib/MultiClick/MultiClick.h>
#include <../Poweroid_SDK_10/lib/DS1307/DS1307.h>


Timings timings = {0};
unsigned long SBY_MILLS = 0L;
TimingState FLASH(750L);
TimingState FLASH_SBY(250L);

#define IND IND_3

MultiClick btn = MultiClick(IN3_PIN);

Context CTX = Context(SIGNATURE, FULL_VERSION, PROPS.FACTORY, PROPS.props_size, ID,
                      PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

void apply_timings() {
    timings.countdown_power.interval = (unsigned long) PROPS.FACTORY[0].runtime * 3600000L +
                                      (unsigned long) PROPS.FACTORY[1].runtime * 60000L;
    SBY_MILLS = static_cast<unsigned long>(PROPS.FACTORY[2].runtime * 60000L);
}

uint16_t banner_value;

void fillBanner() {
    if (state_power == SP_ALARM){
        BANNER.mode = 0;
        sprintf(BANNER.data.text, "%s" , "ALARM");
    } else {
        BANNER.mode = 2;
//        sprintf(BANNER, BANNER_FMT, RTC.get(DS1307_HR, true), RTC.get(DS1307_MIN, false), RTC.get(DS1307_SEC, false));
//        sprintf(BANNER, "L=%dcm", ULTRASONIC.getDistance());
        int16_t val = PWR.SENS->getNormalizedSensor(SEN_2, -100, 0, 102, 920);
        BANNER.data.gauges[0].val = val;
        BANNER.data.gauges[0].min = PROPS.FACTORY[1].runtime;
        BANNER.data.gauges[0].max = PROPS.FACTORY[0].runtime;
        BANNER.data.gauges[0].measure = KPA;

        BANNER.data.gauges[1].val = val;
        BANNER.data.gauges[1].min = PROPS.FACTORY[1].runtime;
        BANNER.data.gauges[1].max = PROPS.FACTORY[0].runtime;
        BANNER.data.gauges[1].measure = KPA;
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
                state_power = SP_PRE_POWER;
                break;
            }
            if (!timings.countdown_power.countdown(true, false, false)) {
                state_power = SP_OFF;
                break;
            }
            break;
        }
        case SP_PRE_POWER: {
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
        case SP_POST_POWER: {
            prev_state_power = SP_POWER;
            if (event == HOLD) {
                state_power = SP_OFF;
                break;
            }
            if (event == CLICK) {
                state_power = SP_POWER;
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
    CMD.printChangedState(prev_state_power, state_power, 0);
}

void setup() {
    PWR.begin();
//    ULTRASONIC.begin();
}

void loop() {

    apply_timings();

    PWR.run();

    run_state_power(btn.checkButton());

    bool power = (state_power == SP_POWER || state_power == SP_PRE_POWER  || state_power == SP_POST_POWER);

    PWR.power(REL_A, power);
    PWR.power(REL_B, power);

    if (power) {
        if (state_power == SP_POWER) {
            INDICATORS.set(IND, true);
        } else {
            INDICATORS.flash(IND, &FLASH, true);
        }

    } else {
        if (state_power == SP_ALARM) {
            INDICATORS.flash(IND, &FLASH_SBY, true);
        } else {
            INDICATORS.set(IND, false);
        }
    }
}
