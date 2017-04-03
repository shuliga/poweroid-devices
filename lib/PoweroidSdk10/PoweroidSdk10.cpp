#include <avr/wdt.h>
#include "PoweroidSdk10.h"

Pwr::Pwr(Context &ctx) : REL(), SENS(), CTX(&ctx), PERS(Persistence(ctx.signature, ctx.RUNTIME, ctx.props_size)), CMD(Commands(*CTX, PERS)), CTRL(Controller(CMD, *CTX)) {
    ctx.SENS = &SENS;
    ctx.RELAYS = &REL;
}

void Pwr::begin() {
    printVersion();
    init_outputs();
    init_inputs();
    SENS.initSensors();
    CTRL.begin();
    BT = new Bt(CTX->id);
    loadDisarmedStates();
    wdt_enable(WDTO_4S);
}

void Pwr::run() {
    wdt_reset();
    CMD.listen();
    CTRL.process();
}

void Pwr::processSensors() {
    SENS.process();
}

void Pwr::printVersion() {
    Serial.print(F(VERSION));
    Serial.print(F("-"));
    Serial.println(F(BOARD_VERSION));
}

void Pwr::init_outputs() {
    for (uint8_t i = 0; i < REL.size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
}

void Pwr::init_inputs() {
    for (uint8_t i = 0; i < SENS.size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
}

void Pwr::loadDisarmedStates() {
    for(uint8_t i = 0; i < CTX->states_size; i++){
        bool disarm = PERS.loadState(i);
        CTX->disarmState(i, disarm);
        if (disarm) {
            CTX-> printState(i);
        }
    }
}
