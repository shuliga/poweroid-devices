//
// Created by SHL on 20.03.2017.
//

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#define FLIP_DISPLAY 1

static const int ONE_LINE = 16;

static const int PROP_SIZE = 4;

#include <MultiClick.h>
#include "commons.h"
#include "context.h"
#include "commander.h"

class Controller {
public:

    Controller(Context &_ctx, Commands &_cmd);

    void begin();

    void process();

private:
    Context *ctx;
    Commands *cmd;

    void initEncoderInterrupts();

    void initDisplay();

    void outputPropDescr(char * _buff);

    void outputStatus(const __FlashStringHelper *txt, const long val);

    void outputPropVal(uint8_t measure_idx, int16_t _prop_val, bool brackets, bool measure);

    void switchDisplay(boolean inverse) const;

    void updateProperty(uint8_t idx) const;

    bool loadProperty(uint8_t idx) const;

    void goToEditProp(uint8_t i) const;

    bool firstRun() const;

    void goToBrowse() const;

    bool testControl(TimingState &timer) const;

    void copyProperty(Property &prop, uint8_t idx) const;

    static void padLine(char *_buff, uint8_t lines, uint8_t tail);

    void outputState() const;

    bool canGoToEdit();

};


#endif //_CONTROLLER_H
