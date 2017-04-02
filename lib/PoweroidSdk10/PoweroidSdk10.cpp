#include <avr/wdt.h>
#include "PoweroidSdk10.h"
#include "pin_io.h"

Pwr::Pwr(Context &ctx) : CTX(&ctx), CMD(Commands(*CTX)), CTRL(Controller(CMD, *CTX)) {
    REL = Relays();
    SENS = ctx.SENS;
}

void Pwr::begin() {
    Serial.begin(9600);
    printVersion();
    init_outputs();
    init_inputs();
    SENS->init_sensors();
    BT = new Bt(CTX->id);
    wdt_enable(WDTO_4S);
}

void Pwr::run() {
    wdt_reset();
    CMD.listen();
    CTRL.process();
}

void Pwr::processSensors() {
    SENS->process();
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
    for (uint8_t i = 0; i < SENS->size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
}