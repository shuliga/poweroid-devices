
#include "pin_io.h"
#include "PoweroidSdk10.h"
#include "poweroid_fan_2x1_1x1_prop.h"


char ID[] = "PWR-FAN-21-11";

Context *CTX;
Pwr *PWR;

Timings timings[2] = {{0, 0, 0},
                      {0, 0, 0}};

State prev_state = POWER_SBY;

void init_timing(uint8_t index) {
    timings[index].countdown_power.interval = (unsigned long) FAN_PROPS._RUNTIME[index][0];
    timings[index].timeAfter_lightOff.interval = (unsigned long) FAN_PROPS._RUNTIME[index][1];
    timings[index].countdown_light.interval = (unsigned long) FAN_PROPS._RUNTIME[index][2];
}


void run_state(bool light, uint8_t power_pin, uint8_t state_id, Timings &timings) {
    switch (states[state_id]) {
        case OFF: {
            prev_state = OFF;
            if (isTimeAfter(timings.countdown_light, light)) {
                states[state_id] = AL;
            }
            break;
        }
        case AL: {
            prev_state = AL;
            if (isTimeAfter(timings.timeAfter_lightOff, !light)) {
                states[state_id] = AD;
            }
            break;
        }
        case AD: {
            prev_state = AD;
            timings.countdown_power.suspended = 0;
            states[state_id] = POWER;
            break;
        }
        case POWER: {
            bool firstRun = prev_state != POWER;
            prev_state = POWER;
            if (countdown(timings.countdown_power, firstRun, false, false)) {
                if (light) {
                    states[state_id] = POWER_SBY;
                }

            } else {
                states[state_id] = OFF;
            }
            break;
        }
        case POWER_SBY: {
            bool firstRun = prev_state != POWER_SBY;
            prev_state = POWER_SBY;
            if (countdown(timings.countdown_power, false, true, false)) {
                if (!light) {
                    states[state_id] = POWER;
                }
                if (firstRun){
                    isTimeAfter(timings.countdown_light, false); //reset timer
                }
                if (isTimeAfter(timings.countdown_light, light)) {
                    states[state_id] = AL;
                }
            }
            break;
        }

    }
    if (prev_state != states[state_id]) {
        Serial.println(printState(state_id));
    }
    led(13, states[state_id] == POWER);

    pin_inv(power_pin, states[state_id] == POWER);

}

void setup() {
    Serial.begin(9600);
    CTX = new Context{SIGNATURE, version, new Sensors(), FAN_PROPS.FACTORY, FAN_PROPS.RUNTIME, FAN_PROPS.props_size,
                      (char *) &ID, ARRAY_SIZE(states), printState};
    PWR = new Pwr(CTX);
    PWR->printVersion();
    PWR->begin();
}

void loop() {

    init_timing(0);
    init_timing(1);

    PWR->processSensors();

    bool light1 = PWR->SENS->is_sensor_on(0);
    bool light2 = PWR->SENS->is_sensor_on(1);
    bool light3 = PWR->SENS->is_sensor_on(2);

    run_state(light1, PWR1_PIN, 0, timings[0]);
    run_state(light2 || light3, PWR2_PIN, 1, timings[1]);

    PWR->run();
}

