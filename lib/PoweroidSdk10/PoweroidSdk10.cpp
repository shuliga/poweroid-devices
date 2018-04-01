#include "global.h"
#include <avr/wdt.h>
#include "PoweroidSdk10.h"

Pwr::Pwr(Context &ctx, Commands *_cmd, Controller *_ctrl, Bt *_bt) : CTX(&ctx), CMD(_cmd), CTRL(_ctrl), BT(_bt) {
    REL = &ctx.RELAYS;
    SENS = &ctx.SENS;
}

void Pwr::begin() {
    Serial.begin(DEFAULT_BAUD);
#ifdef SSERIAL
    SSerial.begin(DEFAULT_BAUD);
    SSerial.println("SSerial-started");
#endif
    printVersion();

    init_pins();

    CTX->PERS.begin();

    SENS->initSensors();

    loadDisarmedStates();

    if (BT){
        BT->begin();
    }
#ifndef NO_CONTROLLER
    if (CTRL) {
        CTRL->begin();
    }
#endif
#ifdef WATCH_DOG
    wdt_enable(WDTO_8S);
#endif
}

void Pwr::run() {
#ifdef WATCH_DOG
    wdt_reset();
#endif
    SENS->process();

    if (CMD) {
        CMD->listen();
    }

    if (BT){
        CTX->passive = !BT->server;
        REL->mapped = !CTX->passive;
        bool newConnected = BT->getConnected();
        CTX->refreshState = newConnected != CTX->connected;
        CTX->connected = newConnected;
    }

#ifndef NO_CONTROLLER
    if (CTRL) {
        CTRL->process();
    }
#endif
    if (firstRun) {
        writeLog('I', SIGNATURE, 101);
        firstRun = false;
    }
}

void Pwr::printVersion() {
    Serial.println(CTX->version);
}

void Pwr::init_pins() {
    for (uint8_t i = 0; i < REL->size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
    REL->reset();

    for (uint8_t i = 0; i < SENS->size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
}

void Pwr::loadDisarmedStates() {
    for (uint8_t i = 0; i < state_count; i++) {
        bool disarm = CTX->PERS.loadState(i);
        disarmState(i, disarm);
        if (disarm) {
            Serial.println(printState(i, BUFF));
        }
    }
}

void Pwr::power(uint8_t i, bool power) {
    CTX->refreshState = true;
    REL->power(i, power);
}
