#include <states.h>
#include <Poweroid10.h>
#include "poweroid_test_prop.h"

#define ID "PWR_TEST"

TimingState countdown(1000);

Context CTX(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

int8_t counter = 0;

const uint8_t state_count = 0;
StateHolderBase* run_states[state_count] = {};

void runPowerStates() {
}

void applyTimings() {
}

void processSensors() {
}

void fillOutput() {
    BANNER.mode = 0;
    strcpy(BANNER.data.text, PWR.SENS->printDht());
}


void setup() {
    PWR.begin();
    Serial.println("PWR TEST initiated");
}

void loop() {
    PWR.run();
    if(countdown.ping()){
        PWR.REL->power(REL_A, false);
#ifndef MINI
        PWR.REL->power(REL_B, false);
#endif
        INDICATORS.set(IND_1, false);
        INDICATORS.set(IND_2, false);
#ifndef MINI
        INDICATORS.set(IND_3, false);
#endif
        Serial.println(counter);
        switch (counter) {
            case 0:
            {
                PWR.REL->power(REL_A, true);
                Serial.println("Relay A ON");
                break;
            }
#ifndef MINI
            case 1:
            {
                PWR.REL->power(REL_B, true);
                Serial.println("Relay B ON");
                break;
            }
#endif
            case 2:
            {
                INDICATORS.set(IND_1, true);
                Serial.flush();
                Serial.println("Indicator 1 ON");
                break;
            }
            case 3:
            {
                INDICATORS.set(IND_2, true);
                Serial.println("Indicator 2 ON");
#ifdef MINI
                counter = -1;
#endif
                break;
            }
#ifndef MINI
            case 4:
            {
                INDICATORS.set(IND_3, true);
                Serial.println("Indicator 3 ON");
                counter = -1;
                break;
            }
#endif
        }
        counter++;
    }
}