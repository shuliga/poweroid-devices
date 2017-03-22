
#include <Arduino.h>
#include "pin_io.h"
#include "commands.h"
#include "PoweroidSdk10.h"
#include "poweroid_fan_2x1_1x1_prop.h"

#define ENC_BTN_PIN 7
// #define BT

char ID[] = "PWR-FAN-21-11\0";

Context *CTX;
Pwr *PWR;

Timings timings[2] = {{0, 0, 0}, {0, 0, 0}};

void init_timing(uint8_t index){
    timings[index].countdown_power.interval = (unsigned long) FAN_PROPS._RUNTIME[index][0];
    timings[index].timeAfter_lightOff.interval = (unsigned long) FAN_PROPS._RUNTIME[index][1];
    timings[index].countdown_light.interval = (unsigned long) FAN_PROPS._RUNTIME[index][2];
}


void run_relay(bool light, uint8_t  power_pin, uint8_t state_id, Timings *timings){
    State state = states[state_id];
    bool timeAfterLight = isTimeAfter(&timings->countdown_light, light);
    if (state == OFF && timeAfterLight){
        state = AL;
        Serial.println(printState(state, state_id));
    }
    bool timeAfterDark = isTimeAfter(&timings->timeAfter_lightOff, !light);
    if (state == AL && timeAfterDark){
        state = AD;
        Serial.println(printState(state, state_id));
    }

    bool power_on = countdown(&timings->countdown_power, state == AD, light);

    if (state == AD && power_on){
        state = POWER;
        Serial.println(printState(state, state_id));
    }

    if (state == POWER && !power_on){
        state = OFF;
        Serial.println(printState(state, state_id));
    }

    led(13, power_on);

    pin_inv(power_pin, power_on);

}

void setup() {
    Serial.begin(9600);
    CTX = new Context{SIGNATURE, version, new Sensors(), FAN_PROPS.FACTORY, FAN_PROPS.RUNTIME, FAN_PROPS.props_size, (char *) &ID};
    PWR = new Pwr(CTX);
    PWR->printVersion();
    PWR->begin();
}

void loop() {

    init_timing(0);
    init_timing(1);

    CTX->SENS->check_installed();

    bool light1 = CTX->SENS->is_sensor_on(0);
    bool light2 = CTX->SENS->is_sensor_on(1);
    bool light3 = CTX->SENS->is_sensor_on(2);

    run_relay(light1, PWR1_PIN, 0, &timings[0]);
    run_relay(light2 || light3, PWR2_PIN, 1, &timings[1]);

    PWR->run();

}

