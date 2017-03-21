//
// Created by SHL on 20.03.2017.
//

#include <Rotary.h>
#include <MultiClick.h>
#include <ACROBOTIC_SSD1306.h>

#define INCR(val, max) val < max ? val++ : val
#define DECR(val, min) val > min ? val-- : val

#include "commons.h"
#include "commands.h"
#include "controller.h"

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP, SENSORS
} oldState = STORE, state = BROWSE;


static volatile int prop_idx = 0;
static int c_prop_idx = -1;

static volatile long prop_value;
static long c_prop_value = -1;
static long old_prop_value;
static volatile long prop_min;
static volatile long prop_max;

Rotary encoder = Rotary();
MultiClick encoderClick = MultiClick(ENC_BTN_PIN);
TimingState displayTiming = TimingState(1000L);


Controller::Controller(Commands *_cmd, Context *_ctx) : cmd(_cmd), ctx(_ctx) {
    initDisplay();
    initEncoderInterrupts();
}

void Controller::initEncoderInterrupts() {
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
    sei();
    props_idx_max = ctx->props_size - 1;
}

void Controller::initDisplay() {
    if (oled.checkAndInit()) {
        oled.clearDisplay();
        oled.setBrightness(0);
        outputTitle();
    };
}

void Controller::outputTitle() const {
    oled.setTextXY(0, 1);
    oled.putString(ctx->id);
}


void Controller::process() {

    ctx->SENS->updateTnH();

    McEvent event = encoderClick.checkButton();

    switch (state) {
        case EDIT_PROP: {
            if (c_prop_value != prop_value) {
                cli();
                ctx->RUNTIME[prop_idx] = prop_value * ctx->FACTORY[prop_idx].scale;
                c_prop_value = prop_value;
                sei();
                printPropDescr((uint8_t) prop_idx);
                outputPropVal(&ctx->FACTORY[prop_idx], (int16_t) prop_value, true, true);
            }
            if (event == CLICK) {
                c_prop_idx = -1; // invalidate cache;
                c_prop_value = -1; // invalidate cache;
                oldState = state;
                state = BROWSE;
            };
            if (event == HOLD) {
                oldState = state;
                state = STORE;
            };
            break;
        }
        case BROWSE: {
            if (c_prop_idx != prop_idx) {
                cli();
                prop_value = (ctx->RUNTIME[prop_idx] / ctx->FACTORY[prop_idx].scale);
                prop_min = (ctx->FACTORY[prop_idx].minv / ctx->FACTORY[prop_idx].scale);
                prop_max = (ctx->FACTORY[prop_idx].maxv / ctx->FACTORY[prop_idx].scale);
                c_prop_idx = prop_idx;
                sei();

                printPropDescr(prop_idx);
                outputPropVal(&ctx->FACTORY[prop_idx], prop_value, false, true);
            }
            if (event == CLICK) {
                c_prop_value = -1; // invalidate cache;
                c_prop_idx = -1; // invalidate cache;
                old_prop_value = prop_value; // for reminder in status
                oldState = state;
                state = EDIT_PROP;
            };
            if (event == HOLD) {
                oldState = state;
                state = SLEEP;
            };
            break;
        }
        case STORE: {
            cmd->storeProps();
            c_prop_value = -1; // invalidate cache;
            c_prop_idx = -1; // invalidate cache;
            oldState = state;
            state = BROWSE;
            break;
        }
        case SLEEP: {
            if (oldState != state) {
                oldState = state;

                oled.displayOff();
                oled.setNormalDisplay();
                oled.clearDisplay();
                oled.displayOn();
            }

            outputSleepScreen();

            if (event == CLICK) {
                oled.displayOff();
                oled.setInverseDisplay();
                oled.clearDisplay();
                oled.displayOn();
                outputTitle();

                c_prop_idx = -1; // invalidate cache;
                state = BROWSE;
            };
            break;
        }
    }

    if (oldState != state) {
        switch (state) {
            case EDIT_PROP:
                outputStatus(F("edit value:"), old_prop_value);
                break;
            case BROWSE:
                outputStatus(F("property:  "), prop_idx);
                break;
            case STORE:
                outputStatus(F("<storing..>"), prop_value);
                delay(500);
                break;
        }
    }

}

void Controller::outputSleepScreen() {
    if (ping(&displayTiming)) {
        float temp = ctx->SENS->getTemperature();
        float humid = ctx->SENS->getHumidity();
        char out[12];
        char spc[] = "C, ";
        strcpy(out, String(temp, 0).c_str());
        strcat(out, spc);
        strcat(out, String(humid, 0).c_str());
        strcat(out, "%");
        oled.outputTextXY(3, 64, out, true);
    }
}

void Controller::printPropDescr(uint8_t _idx) {
    if (oled.isConnected()) {
        oled.setTextXY(1, 0);
        oled.putString(ctx->FACTORY[_idx].desc);
        oled.putString("      ");
    }
}

void Controller::outputStatus(const __FlashStringHelper *txt, const long val) {
    oled.setTextXY(7, 0);
    oled.putString(txt);
    oled.setTextXY(7, 12);
    oled.putNumber(val);
    oled.putString("  ");
}

void Controller::outputPropVal(Property *_prop, int16_t _prop_val, bool brackets, bool measure) {
    char str_text[12];
    String fmt = brackets && measure ? "[%i]%s" : (brackets & !measure ? "[%i]" : (!brackets && measure ? "%i%s" : "%i"));
    String measure_s = String(_prop->desc);
    sprintf(str_text, fmt.c_str(), _prop_val, measure_s.substring(measure_s.lastIndexOf('(') + 1, measure_s.lastIndexOf(')')).c_str());
    oled.outputTextXY(3, 64, str_text, true);
}


ISR(PCINT2_vect) {
    unsigned char result = encoder.process();
    if (result != NOTHING) {
        switch (state) {
            case EDIT_PROP: {
                result == DIR_CW ? DECR(prop_value, prop_min) : INCR(prop_value, prop_max);
                break;
            }
            case BROWSE: {
                result == DIR_CW ? DECR(prop_idx, 0) : INCR(prop_idx, props_idx_max);
                break;
            }
            case STORE:
                break;
        }
    }
}