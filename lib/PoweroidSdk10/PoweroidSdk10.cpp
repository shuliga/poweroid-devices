#include <avr/wdt.h>
#include "PoweroidSdk10.h"

Pwr::Pwr(Context &ctx, Commands *_cmd, Controller *_ctrl, Bt *_bt) : CTX(&ctx), CMD(_cmd), CTRL(_ctrl), BT(_bt) {
    REL = &ctx.RELAYS;
    SENS = &ctx.SENS;
}

void Pwr::begin() {

    Serial.begin(9600);
#ifdef SSERIAL
    SSerial.begin(9600);
    SSerial.println("SSerial started");
#endif
    printVersion();

    init_pins();

    CTX->PERS.begin();

    SENS->initSensors();

    loadDisarmedStates();

    if (BT){
        BT->begin();
    }
#ifndef SSERIAL
    if (CTRL) {
        CTRL->begin();
    }
#endif
//    wdt_enable(WDTO_8S);
}

void Pwr::run() {
//    wdt_reset();
    SENS->process();

    if (BT){
        CTX->passive = BT->getPassive();
    }

    if (CMD) {
        CMD->listen();
    }
#ifndef SSERIAL
    if (CTRL) {
        CTRL->process();
    }
#endif
}

void Pwr::printVersion() {
    Serial.println(F(FULL_VERSION));
}

void Pwr::init_pins() {
    for (uint8_t i = 0; i < REL->size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
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
