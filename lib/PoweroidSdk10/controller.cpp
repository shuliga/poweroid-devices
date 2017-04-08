//
// Created by SHL on 20.03.2017.
//


#define INCR(val, max) val < max ? val++ : val
#define DECR(val, min) val > min ? val-- : val

#include <Rotary.h>
#include <ACROBOTIC_SSD1306.h>
#include "commons.h"
#include "commands.h"
#include "controller.h"

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP, SENSORS
} oldState = STORE, state = BROWSE;

static TimingState sleep_timer = TimingState(100000L);

static volatile bool control_touched = false;
static volatile uint8_t prop_idx = 0;
static volatile long prop_value;

static int8_t c_prop_idx = -1;
static long c_prop_value = -1;
static long old_prop_value;
static bool prop_changed;

static volatile long prop_min;
static volatile long prop_max;

#ifdef ENC1_PIN
#ifdef ENC2_PIN
Rotary encoder = Rotary(ENC1_PIN, ENC2_PIN);
#endif
#endif

MultiClick encoderClick = MultiClick(ENC_BTN_PIN);
TimingState displayTiming = TimingState(1000L);
TimingState displaySearchTiming = TimingState(2000L);


Controller::Controller(Commands &_cmd, Context &_ctx) : cmd(&_cmd), ctx(&_ctx) {}

void Controller::begin() {
    initDisplay();
#ifdef ENC1_PIN
#ifdef ENC2_PIN
    initEncoderInterrupts();
#endif
#endif
}

void Controller::initEncoderInterrupts() {
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
    sei();
    props_idx_max = ctx->props_size - 1;
}

void Controller::initDisplay() {
    if (oled.checkAndInit(FLIP_DISPLAY)) {
        oled.clearDisplay();
        oled.setBrightness(0);
        outputTitle();
    };
}

void Controller::outputTitle() const {
    char _cbuff[17];
    oled.setTextXY(7, 0);
    strlcpy(_cbuff, ctx->id, 17);
    oled.putString(_cbuff);
}


void Controller::process() {

    McEvent event = encoderClick.checkButton();

    switch (state) {
        case EDIT_PROP: {
            oldState = state;
            if (c_prop_value != prop_value || ctx->invalidate) {
                cli();
                ctx->RUNTIME[prop_idx] = prop_value * ctx->FACTORY[prop_idx].scale;
                c_prop_value = prop_value;
                sei();
                printPropDescr((uint8_t) prop_idx);
                outputPropVal(ctx->FACTORY[prop_idx], (int16_t) prop_value, true, true);
                outputStatus(F("edit value:"), old_prop_value);
            }
            if (event == CLICK) {
                c_prop_idx = -1; // invalidate cache;
                c_prop_value = -1; // invalidate cache;
                state = BROWSE;
            };
            if (event == HOLD) {
                state = STORE;
            };
            break;
        }
        case BROWSE: {
            bool firstRun = oldState != state;
            oldState = state;

            if (control_touched || firstRun) {
                sleep_timer.reset();
                control_touched = false;
            }

            if (c_prop_idx != prop_idx || ctx->invalidate) {
                cli();
                long scale = ctx->FACTORY[prop_idx].scale;
                prop_value = (ctx->RUNTIME[prop_idx] / scale);
                prop_min = (ctx->FACTORY[prop_idx].minv / scale);
                prop_max = (ctx->FACTORY[prop_idx].maxv / scale);
                c_prop_idx = prop_idx;
                sei();

                printPropDescr(prop_idx);
                outputPropVal(ctx->FACTORY[prop_idx], prop_value, false, true);
                outputStatus(F("property:  "), prop_idx);
                prop_changed = false;
            }

            if (event == CLICK) {
                c_prop_value = -1; // invalidate cache;
                c_prop_idx = -1; // invalidate cache;
                old_prop_value = prop_value; // for reminder in status
                state = EDIT_PROP;
            };

            if (event == HOLD || sleep_timer.isTimeAfter(true)) {
                state = SLEEP;
            };

            break;
        }
        case STORE: {
            oldState = state;
            cmd->storeProps();
            outputStatus(F("<storing..>"), prop_value);
            delay(500);
            c_prop_value = -1; // invalidate cache;
            c_prop_idx = -1; // invalidate cache;
            state = BROWSE;
            break;
        }
        case SLEEP: {
            bool firstRun = oldState != state;
            oldState = state;

            if (firstRun) {
                sleep_timer.reset();
                switchDisplay(false);
            }

            outputSleepScreen(sleep_timer.isTimeAfter(true));

            exitSleepOnClick(event);
            break;
        }
    }
    ctx->invalidate = false;
    detectDisplay();
}

void Controller::exitSleepOnClick(const McEvent &event) const {
    if (event == CLICK || control_touched) {
        control_touched = false;
        switchDisplay(true);
        outputTitle();
        c_prop_idx = -1; // invalidate cache;
        sleep_timer.reset();
        state = BROWSE;
    };
}

void Controller::switchDisplay(boolean inverse) const {
    oled.displayOff();
    inverse ? oled.setInverseDisplay() : oled.setNormalDisplay();
    oled.clearDisplay();
    oled.displayOn();
}

void Controller::detectDisplay() {
    if (displaySearchTiming.ping() && !oled.getConnected()) {
        initDisplay();
    }
}

void Controller::outputSleepScreen(bool dither) {
    if (displayTiming.ping() && oled.getConnected()) {
        oled.outputTextXY(3, 64, ctx->SENS->printDht(), true, dither);
    }
}

void Controller::printPropDescr(uint8_t _idx) {
    if (oled.getConnected()) {
        oled.setTextXY(1, 0);
        oled.putString(ctx->FACTORY[_idx].desc);
        oled.putString(F("      "));
    }
}

void Controller::outputStatus(const __FlashStringHelper *txt, const long val) {
    oled.setTextXY(0, 0);
    oled.putString(txt);
    oled.setTextXY(0, 12);
    oled.putNumber(val);
    oled.putString("  ");
}

void Controller::outputPropVal(Property &_prop, uint16_t _prop_val, bool brackets, bool _measure) {
    char str_text[12];
    char measure_c[6];
    const char *fmt =
            brackets && _measure ? "[%i]%s" : (brackets & !_measure ? "[%i]" : (!brackets && _measure ? "%i%s"
                                                                                                      : "%i"));
    PGM_P p = reinterpret_cast<PGM_P>(_prop.desc);
    size_t n = 0;
    uint8_t i = 0;
    bool start = false;
    while (i < 11) {
        unsigned char c = (unsigned char) pgm_read_byte(p++);
        if (c == ')') break;
        if (start) {
            measure_c[i] = c;
            i++;
        }
        if (c == '(') start = true;
    }
    measure_c[i] = 0;
    sprintf(str_text, fmt, _prop_val, measure_c);
    oled.outputTextXY(3, 64, str_text, true, false);
}


#ifdef ENC1_PIN
#ifdef ENC2_PIN

ISR(PCINT2_vect) {
    unsigned char result = encoder.process();
    if (result != NOTHING) {
        control_touched = true;
        switch (state) {
            case EDIT_PROP: {
                result == DIR_CW ? DECR(prop_value, prop_min) : INCR(prop_value, prop_max);
                break;
            }
            case BROWSE: {
                result == DIR_CW ? DECR(prop_idx, 0) : INCR(prop_idx, props_idx_max);
                break;
            }
        }
    }
}

#endif
#endif
