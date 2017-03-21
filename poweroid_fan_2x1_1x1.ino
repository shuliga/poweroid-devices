
#include <Arduino.h>
#include "commands.h"
#include "bluetooth.h"
#include "controller.h"
#include "PoweroidSdk10.h"
#include "poweroid_fan_2x1_1x1_prop.h"

#define ENC_BTN_PIN 7
// #define BT

char ID[] = "PWR-FAN-21-11\0";

Timings timings[2];

Bt *bt;
Commands *CMD;
Controller *CTRL;

static Context CTX = {FAN_PROPS.FACTORY, FAN_PROPS.RUNTIME, FAN_PROPS.props_size, states, ARRAY_SIZE(states),
                      (char *) &ID};

void init_timing(int index){
    timings[index].countdown_power.interval = (unsigned long) FAN_PROPS._RUNTIME[index][0];
    timings[index].timeAfter_lightOff.interval = (unsigned long) FAN_PROPS._RUNTIME[index][1];
    timings[index].countdown_light.interval = (unsigned long) FAN_PROPS._RUNTIME[index][2];
}


void run_relay(bool light, int power_pin, int state_id, Timings *timings){
    String *_state = &states[state_id];
    bool timeAfterLight = isTimeAfter(&timings->countdown_light, light);
    if (*_state == "OFF" && timeAfterLight){
        *_state = "AL";
        Serial.println(printState(states, state_id));
    }
    bool timeAfterDark = isTimeAfter(&timings->timeAfter_lightOff, !light);
    if (*_state == "AL" && timeAfterDark){
        *_state = "AD";
        Serial.println(printState(states, state_id));
    }

    bool power_on = countdown(&timings->countdown_power, *_state == "AD", light);

    if (*_state == "AD" && power_on){
        *_state = "POWER";
        Serial.println(printState(states, state_id));
    }

    if (*_state == "POWER" && !power_on){
        *_state = "OFF";
        Serial.println(printState(states, state_id));
    }

    led(12, timeAfterLight);
//  led(12, timeAfterDark);
    led(13, power_on);
//  flash(&flash_333, 13 , power_on);
    pin_inv(power_pin, power_on);

}

void setup() {

    init_system();
    init_sensors();

    Serial.begin(9600);
    printVersion();

#ifdef  BT
    bt = new Bt(ID);
#endif
    CMD = new Commands(&CTX);
    CTRL = new Controller(CMD, &CTX);
}

void loop() {

    init_timing(0);
    init_timing(1);

    check_installed();

    bool light1 = is_sensor_on(0);
    bool light2 = is_sensor_on(1);
    bool light3 = is_sensor_on(2);

    run_relay(light1, PWR1_PIN, 0, &timings[0]);
    run_relay(light2 || light3, PWR2_PIN, 1, &timings[1]);

    CMD->listen();
    CTRL->process();

}

