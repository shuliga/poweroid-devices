//
// Created by SHL on 20.03.2017.
//

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "commons.h"
#include "commands.h"
#include "sensors.h"

static volatile int props_idx_max = 0;

class Controller {
public:
    Controller(Commands * _cmd, Context * _ctx);
    void process();
private:
    Context *ctx;
    Commands *cmd;

    void initEncoderInterrupts();
    void initDisplay();
    void printPropDescr(uint8_t _idx);
    void outputStatus(const __FlashStringHelper *txt, const long val);
    void outputPropVal(Property *_prop, uint16_t _prop_val, bool brackets, bool _measure);

    void outputSleepScreen();

    void outputTitle() const;
};


#endif //_CONTROLLER_H
