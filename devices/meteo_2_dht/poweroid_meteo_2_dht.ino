#include <states.h>
#include <Poweroid10.h>
#include "poweroid_meteo_2_dht_prop.h"
#include "poweroid_meteo_2_dht_state.h"

#define ID "PWR_METEO_2"

#define RAIN_SEN SEN_2

Context CTX(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

bool static raining = false;
bool static dhtOk = false;

void runRainState() {
    switch (stateHolderRain.state) {
        case SR_DRY: {
            stateHolderRain.firstEntry();
            if (raining)
                stateHolderRain.gotoState(SR_RAINING);
            break;
        }
        case SR_RAINING: {
            stateHolderRain.firstEntry();
            if (!raining)
                stateHolderRain.gotoState(SR_DRY);
            break;
        }
        case SR_DISARM: {
            stateHolderRain.firstEntry();
            break;
        }

    }
};

void runSystemState() {
    switch (stateHolderSystem.state) {
        case SG_OPERATING: {
            if (!stateHolderSystem.firstEntry()){
                if (!dhtOk)
                    stateHolderSystem.gotoState(SG_ALARM);
            }
            break;
        }
        case SG_ALARM: {
            stateHolderSystem.firstEntry();
            if (dhtOk)
                stateHolderSystem.gotoState(SG_OPERATING);
            break;
        }
        case SG_DISARM: {
            stateHolderSystem.firstEntry();
            break;
        }
    }
};

void runPowerStates() {
    runSystemState();
    runRainState();
}

void applyTimings() {
}

void processSensors() {
    raining = CTX.SENS.isSensorOn(RAIN_SEN);
    dhtOk = CTX.SENS.isDhtOperating();
}

void fillOutput() {
    BANNER.mode = 0;
    strcpy(BANNER.data.text, PWR.SENS->printDht());
}

void setup() {
    PWR.begin();
}

void loop() {
    PWR.run();

    bool rainSignal = stateHolderRain.state == SR_RAINING;
    bool alarm = stateHolderSystem.state == SG_ALARM;
    bool operating = stateHolderSystem.state == SG_OPERATING;

    PWR.power(REL_A, rainSignal & operating);

    INDICATORS.flash(IND_1, flash_symm(timerCounter_4Hz), alarm);
}