
#include <Arduino.h>
#include "pin_io.h"
#include "PoweroidSdk10.h"
#include "poweroid_fan_3x1_dht_prop.h"


char ID[] = "PWR-FAN-31-DHT";

Context *CTX;
Pwr *PWR;

Timings timings = {0, 0, 0, 0, 0};

State prev_state = POWER_SBY;

void init_timing() {
    timings.countdown_power.interval = (unsigned long) FAN_PROPS.RUNTIME[0];
    timings.delay_power.interval = (unsigned long) FAN_PROPS.RUNTIME[1];
    timings.light1_standby.interval = (unsigned long) FAN_PROPS.RUNTIME[2];
    timings.light2_standby.interval = (unsigned long) FAN_PROPS.RUNTIME[3];
    timings.humidity_timeout.interval = (unsigned long) FAN_PROPS.RUNTIME[4];
}


void run_state(bool light, bool humidity, uint8_t power_pin, uint8_t state_id, Timings *timings) {
    switch (states[state_id]) {
        case OFF: {
            prev_state = OFF;
            if (isTimeAfter(&timings->light1_standby, light)) {
                states[state_id] = AL;
            }
            if(humidity && !light){
                states[state_id] = POWER;
            }
            break;
        }
        case AL: {
            prev_state = AL;
            if (isTimeAfter(&timings->delay_power, !light)) {
                states[state_id] = AD;
            }
            break;
        }
        case AD: {
            prev_state = AD;
            timings->countdown_power.suspended = 0;
            states[state_id] = POWER;
            break;
        }
        case POWER: {
            bool firstRun = prev_state != POWER;
            prev_state = POWER;
            if (countdown(&timings->countdown_power, firstRun, false, false) || countdown(&timings->humidity_timeout, firstRun, false, !humidity)) {
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
            if (countdown(&timings->countdown_power, false, true, false) || countdown(&timings->humidity_timeout, false, true, false)) {
                if (!light) {
                    states[state_id] = POWER;
                }
                if (firstRun){
                    isTimeAfter(&timings->light1_standby, false); //reset timer
                }
                if (isTimeAfter(&timings->light1_standby, light)) {
                    states[state_id] = AL;
                }
            }
            break;
        }

    }
    if (prev_state != states[state_id]) {
        Serial.println(printState(states[state_id], state_id));
    }
    led(13, states[state_id] == POWER);

    pin_inv(power_pin, states[state_id] == POWER);

}

void setup() {
    Serial.begin(9600);
    CTX = new Context{SIGNATURE, version, new Sensors(), FAN_PROPS.FACTORY, FAN_PROPS.RUNTIME, FAN_PROPS.props_size,
                      (char *) &ID};
    PWR = new Pwr(CTX);
    PWR->printVersion();
    PWR->begin();
}

void loop() {

    init_timing();

    PWR->processSensors();

    bool light1 = PWR->SENS->is_sensor_on(1);
    bool light2 = PWR->SENS->is_sensor_on(2);
    bool humidity = PWR->SENS->getHumidity() > FAN_PROPS.RUNTIME[5];

    run_state(light1 || light2, humidity, PWR1_PIN, 0, &timings);

    PWR->run();

}

