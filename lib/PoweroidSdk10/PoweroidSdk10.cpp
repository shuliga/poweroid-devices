#include "PoweroidSdk10.h"
#include "pin_io.h"

Pwr::Pwr(Context *ctx) : CTX(ctx), CMD(Commands(*CTX)), CTRL(Controller(CMD, *CTX)) {
    REL = Relays();
    SENS = ctx->SENS;
}

void Pwr::begin() {
    init_outputs();
    init_inputs();
    SENS->init_sensors();
}

void Pwr::run() {
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