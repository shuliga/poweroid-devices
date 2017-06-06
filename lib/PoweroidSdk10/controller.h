//
// Created by SHL on 20.03.2017.
//

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#define FLIP_DISPLAY 1

#include "commons.h"
#include "context.h"
#include <MultiClick.h>
#include "commands.h"
#include "sensors.h"

static volatile int props_idx_max = 0;

class Controller {
public:
    bool passive = false;

    Controller(Commands &_cmd, Context &_ctx);

    void process();

    void begin();

private:
    Context *ctx;
    Commands *cmd;

    void initEncoderInterrupts();

    void initDisplay();

    void outputPropDescr(uint8_t _idx);

    void outputStatus(const __FlashStringHelper *txt, const long val);

    void outputPropVal(Property &_prop, uint16_t _prop_val, bool brackets, bool _measure);

    void outputSleepScreen(bool dither);

    void outputTitle() const;

    void switchDisplay(boolean inverse) const;

    void exitSleepOnClick(const McEvent &event) const;

    void detectDisplay();

    void updateProperty(uint8_t idx) const;

    void loadProperty(uint8_t idx) const;

    bool testSerialConnection() const;
};


#endif //_CONTROLLER_H
