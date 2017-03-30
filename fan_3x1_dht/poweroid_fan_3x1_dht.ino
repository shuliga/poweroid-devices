
#define ID "PWR-FAN-FLR-32-DHT"

#include "pin_io.h"
#include "bluetooth.h"
#include "PoweroidSdk10.h"
#include "poweroid_fan_3x1_dht_prop.h"
#include "../lib/PoweroidSdk10/commons.h"

char c_ID[] = ID;

Context *CTX;
Pwr *PWR;

Timings timings = {0, 0, 0, 0, 0, 0};

StateLight prev_state_light = SL_DISARM;
StateHumid prev_state_humid = SH_DISARM;
StateTemp prev_state_temp = ST_DISARM;

void init_timing() {
    timings.countdown_power.interval = (unsigned long) FAN_PROPS.RUNTIME[0];
    timings.delay_power.interval = (unsigned long) FAN_PROPS.RUNTIME[1];
    timings.light1_standby.interval = (unsigned long) FAN_PROPS.RUNTIME[2];
    timings.humidity_runtime.interval = (unsigned long) FAN_PROPS.RUNTIME[4];
    timings.humidity_delay.interval = (unsigned long) FAN_PROPS.RUNTIME[5];
    timings.temperature_delay.interval = (unsigned long) FAN_PROPS.RUNTIME[7];
}


void run_state_light(bool light, StateLight &state, Timings &timings) {
    switch (state) {
        case SL_OFF: {
            prev_state_light = SL_OFF;
            if (isTimeAfter(timings.light1_standby, light)) {
                state = AL;
            }
            break;
        }
        case AL: {
            prev_state_light = AL;
            if (isTimeAfter(timings.delay_power, !light)) {
                timings.countdown_power.reset();
                state = SL_POWER;
            }
            break;
        }
        case SL_POWER: {
            bool firstRun = prev_state_light != SL_POWER;
            prev_state_light = SL_POWER;
            if (countdown(timings.countdown_power, firstRun, false, false)) {
                if (light) {
                    state = SL_POWER_SBY;
                }

            } else {
                timings.countdown_power.reset();
                state = SL_OFF;
            }
            break;
        }
        case SL_POWER_SBY: {
            bool firstRun = prev_state_light != SL_POWER_SBY;
            prev_state_light = SL_POWER_SBY;
            if (countdown(timings.countdown_power, false, true, false)) {
                if (!light) {
                    state = SL_POWER;
                }
                if (firstRun) {
                    timings.light1_standby.reset();
                }
                if (isTimeAfter(timings.light1_standby, light)) {
                    state = AL;
                }
            }
            break;
        }

    }

    if (prev_state_light != state) {
        printState(0);
    }
}

void run_state_humid(bool light, bool humidity, StateHumid &state, Timings &timings) {
    switch (state) {
        case SH_OFF: {
            prev_state_humid = SH_OFF;
            if (humidity) {
                state = AH;
            }
            break;
        }
        case AH: {
            prev_state_humid = AH;
            if (isTimeAfter(timings.humidity_delay, humidity)) {
                state = SH_POWER;
            }
            if (!humidity) {
                state = SH_OFF;
            }
            break;
        }
        case SH_POWER: {
            bool firstRun = prev_state_humid != SH_POWER;
            prev_state_humid = SH_POWER;
            if (!countdown(timings.humidity_runtime, firstRun, false, !humidity)) {
                timings.humidity_runtime.reset();
                state = SH_OFF;
            }
            break;
        }
    }
    if (prev_state_humid != state) {
        printState(1);
    }
}

void run_state_temp(bool temperature, StateTemp &state, Timings &timings) {
    switch (state) {
        case ST_OFF: {
            prev_state_temp = ST_OFF;
            if (isTimeAfter(timings.temperature_delay, temperature)) {
                state = ST_POWER;
            }
            break;
        }
        case ST_POWER: {
            bool firstRun = prev_state_temp != ST_POWER;
            prev_state_temp = ST_POWER;
            if (isTimeAfter(timings.temperature_delay, !temperature)) {
                state = ST_OFF;
            }
            break;
        }
    }
    if (prev_state_temp != state) {
        printState(2);
    }
}

void setup() {
    delay(600);
    Serial.begin(9600);
    BT = new Bt(c_ID);
    CTX = new Context{SIGNATURE, version, new Sensors(), FAN_PROPS.FACTORY, FAN_PROPS.RUNTIME, FAN_PROPS.props_size,
                      (char *) &c_ID, state_count, printState};
    PWR = new Pwr(CTX);
    PWR->printVersion();
    PWR->begin();
}

void loop() {

    init_timing();

    PWR->processSensors();

    bool light1 = PWR->SENS->is_sensor_on(1);
    bool light2 = PWR->SENS->is_sensor_on(2);
    bool humidity = PWR->SENS->getHumidity() > GET_PROP_NORM(3);
    bool temperature = PWR->SENS->getTemperature() < GET_PROP_NORM(6);

    run_state_light(light1 || light2, state_light, timings);
    run_state_humid(light1 || light2, humidity, state_humid, timings);
    run_state_temp(temperature, state_temp, timings);

    bool fan_power = (state_light == SL_POWER || state_humid == SH_POWER) &&
                     (state_light != SL_POWER_SBY && state_light != AL);
    bool floor_power = state_temp == ST_POWER;

    PWR->REL.power(0, fan_power);
    PWR->REL.power(1, floor_power);

    led(LED_PIN, fan_power || floor_power);

    PWR->run();

}

