#include "PoweroidSdk10.h"

Pwr::Pwr(Context *ctx): CTX(ctx), CMD(Commands(CTX)), CTRL(Controller(&CMD, CTX)) {
#ifdef  BT
    bt = new Bt(ID);
#endif
}

void Pwr::begin() {
    init_outputs();
    init_inputs();
    CTX->SENS->init_sensors();
}

void Pwr::run() {
    CMD.listen();
    CTRL.process();
}

void Pwr::printVersion() {
    Serial.println(F(VERSION));
}

void Pwr::init_outputs() {
    for (int i = 0; i < sizeof(OUT_PINS) / sizeof(OUT_PINS[i]); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
}

void Pwr::init_inputs() {
    for (int i = 0; i < sizeof(IN_PINS) / sizeof(IN_PINS[i]); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
}