
#define ID "PWR-FAN-FLR-32-DHT"

#include <SoftwareSerial.h>
#include <Wire.h>
#include <../Poweroid_SDK_10/src/global.h>
#include <../Poweroid_SDK_10/src/Poweroid10.h>
#include "poweroid_fan_3x1_dht_state.h"
#include "poweroid_fan_3x1_dht_prop.h"

Timings timings = {DEBOUNCE_DELAY, 0, 0, 0, 0, 0, 0};
TimingState FLASH(500L);

Context CTX = Context(SIGNATURE, FULL_VERSION, FAN_PROPS.FACTORY, FAN_PROPS.props_size, ID,
                  FAN_PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#if !defined(NO_CONTROLLER)
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

void apply_timings() {
    timings.countdown_power.interval = (unsigned long) FAN_PROPS.FACTORY[0].runtime;
    timings.delay_power.interval = (unsigned long) FAN_PROPS.FACTORY[1].runtime;
    timings.light1_standby.interval = (unsigned long) FAN_PROPS.FACTORY[2].runtime;
    timings.humidity_runtime.interval = (unsigned long) FAN_PROPS.FACTORY[4].runtime;
    timings.humidity_delay.interval = (unsigned long) FAN_PROPS.FACTORY[5].runtime;
    timings.temperature_delay.interval = (unsigned long) FAN_PROPS.FACTORY[7].runtime;
}

void fillBanner() {
    BANNER.mode = 0;
    strcpy(BANNER.data.text, PWR.SENS->printDht());
}

void run_state_light(bool light) {
    switch (state_light) {
        case SL_OFF: {
            prev_state_light = SL_OFF;
            if (timings.light1_standby.isTimeAfter(light)) {
                state_light = AL;
            }
            break;
        }
        case AL: {
            prev_state_light =  AL;
            if (timings.delay_power.isTimeAfter(!light)) {
                timings.countdown_power.reset();
                state_light = SL_POWER;
            }
            break;
        }
        case SL_POWER: {
            bool firstRun = prev_state_light != SL_POWER;
            prev_state_light = SL_POWER;
            if (timings.countdown_power.countdown(firstRun, false, false)) {
                if (timings.debounce_delay.isTimeAfter(light)) {
                    state_light = SL_POWER_SBY;
                }

            } else {
                timings.countdown_power.reset();
                state_light = SL_OFF;
            }
            break;
        }
        case SL_POWER_SBY: {
            bool firstRun = prev_state_light != SL_POWER_SBY;
            prev_state_light = SL_POWER_SBY;
            if (timings.countdown_power.countdown(false, true, false)) {
                if (timings.debounce_delay.isTimeAfter(!light)) {
                    state_light = SL_POWER;
                }
                if (firstRun) {
                    timings.light1_standby.reset();
                }
                if (timings.light1_standby.isTimeAfter(light)) {
                    state_light = AL;
                }
            }
            break;
        }
        case SL_DISARM: {
            prev_state_light = SL_DISARM;
            break;
        }

    }
    CMD.printChangedState(prev_state_light, state_light, 0);
}

void run_state_humid(bool humidity) {
    switch (state_humid) {
        case SH_OFF: {
            prev_state_humid = SH_OFF;
            if (humidity) state_humid = AH;
            break;
        }
        case AH: {
            prev_state_humid = AH;
            if (timings.humidity_delay.isTimeAfter(humidity)) state_humid =  SH_POWER;
            if (!humidity) state_humid = SH_OFF;
            break;
        }
        case SH_POWER: {
            bool firstRun = prev_state_humid != SH_POWER;
            prev_state_humid = SH_POWER;
            if (!timings.humidity_runtime.countdown(firstRun, false, !humidity)) {
                timings.humidity_runtime.reset();
                state_humid = SH_OFF;
            }
            break;
        }
        case SH_DISARM: {
            prev_state_humid = SH_DISARM;
            break;
        }
    }
    CMD.printChangedState(prev_state_humid, state_humid, 1);
}

void run_state_temp(bool temperature) {
    switch (state_temp) {
        case ST_OFF: {
            prev_state_temp = ST_OFF;
            if (timings.temperature_delay.isTimeAfter(temperature))state_temp = ST_POWER;
            break;
        }
        case ST_POWER: {
            prev_state_temp = ST_POWER;
            if (timings.temperature_delay.isTimeAfter(!temperature)) state_temp =  ST_OFF;
            break;
        }
        case ST_DISARM: {
            prev_state_temp = ST_DISARM;
            break;
        }
    }
    CMD.printChangedState(prev_state_temp, state_temp, 2);
}

void setup() {
    PWR.begin();
}

void loop() {

    apply_timings();

    PWR.run();

    bool light1 = PWR.SENS->isSensorOn(1);
    bool light2 = PWR.SENS->isSensorOn(2);
    bool humidity = PWR.SENS->isDhtInstalled() && PWR.SENS->getHumidity() > GET_PROP_NORM(3);
    bool temperature = PWR.SENS->isDhtInstalled() && PWR.SENS->getTemperature() < GET_PROP_NORM(6);

    run_state_light(light1 || light2);
    run_state_humid(humidity);
    run_state_temp(temperature);

    bool fan_power = (state_light == SL_POWER || state_humid == SH_POWER) &&
                     (state_light != SL_POWER_SBY && state_light != AL);
    bool floor_power = state_temp == ST_POWER;

    PWR.power(0, fan_power);
    PWR.power(1, floor_power);

    INDICATORS.flash(0, &FLASH, PWR.REL->isPowered(1));
}
