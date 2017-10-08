#include <avr/wdt.h>
#include "PoweroidSdk10.h"

Pwr::Pwr(Context &ctx, Commands &_cmd, Controller &_ctrl, Bt &_bt) : CTX(&ctx), CMD(_cmd), CTRL(_ctrl), BT(_bt) {
    REL = &ctx.RELAYS;
    SENS = &ctx.SENS;
}

void Pwr::begin() {
    Serial.begin(9600);
    printVersion();
    init_outputs();
    init_inputs();

    if (&BT != NULL){
        BT.begin();
    }

    SENS->initSensors();
    if (&CTRL != NULL) {
        CTRL.begin();
    }

    loadDisarmedStates();
//    wdt_enable(WDTO_8S);
}

void Pwr::run() {
//    wdt_reset();
    if (&CMD != NULL) {
        CMD.listen();
    }
    if (&CTRL != NULL) {
        CTRL.process();
    }
}

void Pwr::processSensors() {
    SENS->process();
}

void Pwr::printVersion() {
    Serial.println(F(FULL_VERSION));
}

void Pwr::init_outputs() {
    for (uint8_t i = 0; i < REL->size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
}

void Pwr::init_inputs() {
    for (uint8_t i = 0; i < SENS->size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
}

void Pwr::loadDisarmedStates() {
    for (uint8_t i = 0; i < CTX->states_size; i++) {
        bool disarm = CTX->PERS.loadState(i);
        CTX->disarmState(i, disarm);
        if (disarm) {
            Serial.println(CTX->printState(i));
        }
    }
}
