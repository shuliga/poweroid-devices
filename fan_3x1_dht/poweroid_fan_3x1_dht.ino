
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

bool light1;
bool light2;
bool humidity;
bool temperature;


void applyTimings() {
    timings.countdown_power.interval = (unsigned long) FAN_PROPS.FACTORY[0].runtime;
    timings.delay_power.interval = (unsigned long) FAN_PROPS.FACTORY[1].runtime;
    timings.light1_standby.interval = (unsigned long) FAN_PROPS.FACTORY[2].runtime;
    timings.humidity_runtime.interval = (unsigned long) FAN_PROPS.FACTORY[4].runtime;
    timings.humidity_delay.interval = (unsigned long) FAN_PROPS.FACTORY[5].runtime;
    timings.temperature_delay.interval = (unsigned long) FAN_PROPS.FACTORY[7].runtime;
}

void fillOutput() {
    BANNER.mode = 0;
    strcpy(BANNER.data.text, PWR.SENS->printDht());
}

void run_state_light(bool light) {
    switch (state_light) {
        case SL_OFF: {
            if (timings.light1_standby.isTimeAfter(light)) {
                gotoStateLight(AL);
            }
            break;
        }
        case AL: {
            if (timings.delay_power.isTimeAfter(!light)) {
                timings.countdown_power.reset();
                gotoStateLight(SL_POWER);
            }
            break;
        }
        case SL_POWER: {
            bool firstRun = prev_state_light != SL_POWER;
            if (timings.countdown_power.countdown(firstRun, false, false)) {
                if (timings.debounce_delay.isTimeAfter(light)) {
                    gotoStateLight(SL_POWER_SBY);
                }

            } else {
                timings.countdown_power.reset();
                gotoStateLight(SL_OFF);
            }
            break;
        }
        case SL_POWER_SBY: {
            bool firstRun = prev_state_light != SL_POWER_SBY;
            if (timings.countdown_power.countdown(false, true, false)) {
                if (timings.debounce_delay.isTimeAfter(!light)) {
                    gotoStateLight(SL_POWER);
                }
                if (firstRun) {
                    timings.light1_standby.reset();
                }
                if (timings.light1_standby.isTimeAfter(light)) {
                    gotoStateLight(AL);
                }
            }
            break;
        }
    }
}

void run_state_humid(bool humidity) {
    switch (state_humid) {
        case SH_OFF: {
            if (humidity) gotoStateHumid(AH);
            break;
        }
        case AH: {
            if (timings.humidity_delay.isTimeAfter(humidity)) gotoStateHumid(SH_POWER);
            if (!humidity) gotoStateHumid(SH_OFF);
            break;
        }
        case SH_POWER: {
            bool firstRun = prev_state_humid != SH_POWER;
            if (!timings.humidity_runtime.countdown(firstRun, false, !humidity)) {
                timings.humidity_runtime.reset();
                gotoStateHumid(SH_OFF);
            }
            break;
        }
    }
}

void run_state_temp(bool temperature) {
    switch (state_temp) {
        case ST_OFF: {
            if (timings.temperature_delay.isTimeAfter(temperature))gotoStateTemp(ST_POWER);
            break;
        }
        case ST_POWER: {
            if (timings.temperature_delay.isTimeAfter(!temperature)) gotoStateTemp(ST_OFF);
            break;
        }
    }
}

void processSensors() {
    light1 = PWR.SENS->isSensorOn(1);
    light2 = PWR.SENS->isSensorOn(2);
    humidity = PWR.SENS->isDhtInstalled() && PWR.SENS->getHumidity() > GET_PROP_NORM(3);
    temperature = PWR.SENS->isDhtInstalled() && PWR.SENS->getTemperature() < GET_PROP_NORM(6);
}

void runPowerStates(){
    run_state_light(light1 || light2);
    run_state_humid(humidity);
    run_state_temp(temperature);

}

void setup() {
    PWR.begin();
}

void loop() {

    PWR.run();

    bool fan_power = (state_light == SL_POWER || state_humid == SH_POWER) &&
                     (state_light != SL_POWER_SBY && state_light != AL);
    bool floor_power = state_temp == ST_POWER;

    PWR.power(0, fan_power);
    PWR.power(1, floor_power);

    INDICATORS.flash(0, &FLASH, PWR.REL->isPowered(1));
}
