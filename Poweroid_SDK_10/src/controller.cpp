//
// Created by SHL on 20.03.2017.
//

#define INCR(val, max) val < (max) ? (val)++ : val
#define DECR(val, min) val > (min) ? (val)-- : val

#include <Wire.h>
#include "global.h"
#include <MultiClick/MultiClick.h>
#include <Rotary/Rotary.h>
#include <ACROBOTIC_SSD1306/ACROBOTIC_SSD1306.h>
#include "controller.h"
#include "commands.h"

#ifndef NO_CONTROLLER

#define DISPLAY_BASE 1
#define DISPLAY_BOTTOM 7

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP, STATES, SUSPEND, FLAG
} oldState = STORE, state = BROWSE;

static TimingState sleep_timer = TimingState(100000L);
static TimingState autoComplete_timer = TimingState(6000L);

static volatile bool control_touched = false;
static volatile uint8_t prop_idx = 0;
static volatile uint8_t state_idx = 0;
static volatile long prop_value;

static volatile int props_idx_max = 0;
static volatile int state_idx_max = 0;


static int8_t c_prop_idx = -1;
static int8_t c_state_idx = -1;
static uint8_t c_flag = 255;
static long c_prop_value = -1;
static long old_prop_value;
static bool dither = false;

static volatile long prop_min;
static volatile long prop_max;
static volatile uint8_t prop_measure;
static volatile bool requestForRefresh = false;


#if defined(ENC1_PIN) && defined(ENC2_PIN)
Rotary encoder = Rotary(ENC1_PIN, ENC2_PIN);
#endif

MultiClick encoderClick = MultiClick(ENC_BTN_PIN);
TimingState displayTiming = TimingState(333L);


Controller::Controller(Context &_ctx, Commander &_cmd) : cmd(&_cmd), ctx(&_ctx) {}

void Controller::begin() {
    initDisplay();
#if defined(ENC1_PIN) && defined(ENC2_PIN)
    initEncoderInterrupts();
    props_idx_max = ctx->props_size - 1;
    state_idx_max = state_count - 1;
#endif
}

void Controller::initEncoderInterrupts() {

    cli();
#if defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) //ARDUINO_AVR_UNO_PRO
#define PCVECT PCINT1_vect
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);
#else //ARDUINO_AVR_PRO
#define PCVECT PCINT2_vect
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
#endif
    sei();

}

void Controller::initDisplay() {
    if (oled.checkAndInit(FLIP_DISPLAY)) {
        oled.clearDisplay();
        oled.setBrightness(0);
    };
}

void Controller::outputState(bool relays) const {
    strcpy(BUFF, relays ? (const char *) ctx->RELAYS.relStatus() : ctx->id);
    padLine(BUFF, 1, 0);
    BUFF[15] = (unsigned char) (ctx->remote ? (((ctx->connected ? CHAR_CONNECTED : CHAR_DISCONNECTED) + (ctx->passive ? 0 : 2))) : '\0');
    oled.setTextXY(0, 0);
    oled.putString(BUFF);
}

void Controller::process() {

    oled.checkConnected();

    McEvent event = encoderClick.checkButton();

    switch (state) {

        case EDIT_PROP: {

            if (testControl(autoComplete_timer) || ctx->refreshProps) {
                if (loadProperty(prop_idx)) {
                    outputPropDescr(BUFF);
                    outputStatus(F("Edit value:"), old_prop_value);
                }
            }

            if (c_prop_value != prop_value || ctx->refreshProps) {
                updateProperty(prop_idx);
                outputPropVal(prop_measure, (int16_t) prop_value, true, true);
            }

            if (event == CLICK || autoComplete_timer.isTimeAfter(true)) {
                goToBrowse();
            }

            if (event == DOUBLE_CLICK) {
                prop_value = old_prop_value;
                updateProperty(prop_idx);
                goToBrowse();
            }

            if (event == HOLD) {
                state = STORE;
            }
            break;
        }

        case BROWSE: {

            bool prop_id_changed = c_prop_idx != prop_idx;

            if (testControl(sleep_timer) || prop_id_changed || ctx->refreshProps || ctx->refreshState) {
                if (loadProperty(prop_idx)) {
                    outputPropDescr(BUFF);
                    outputPropVal(prop_measure, (int16_t) prop_value, false, true);
                    outputStatus(F("Property:"), prop_idx + 1);
                }
            }

            if (event == CLICK && canGoToEdit()) {
                goToEditProp(prop_idx);
            }

            if (event == HOLD || sleep_timer.isTimeAfter(true)) {
                state = SLEEP;
            }

            if (event == DOUBLE_CLICK ) {
                state = ctx->canAccessLocally() ? STATES : FLAG;
            }

            break;
        }

        case STATES: {
            if (testControl(sleep_timer) || c_state_idx != state_idx) {
                strcpy(BUFF, getState(state_idx)->name);
                outputPropDescr(BUFF);
                strcpy(BUFF, getState(state_idx)->state);
                oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
                outputStatus(F("State:"), state_idx);
                c_state_idx = state_idx;
            }

            if (event == HOLD) {
                bool disarm = strcmp(getState(state_idx)->state, "DISARM") != 0;
                disarmState(state_idx, disarm);
                cmd->disarmState(state_idx, disarm);
                c_state_idx = -1;
            }

            if (event == CLICK || sleep_timer.isTimeAfter(true)) {
                state = BROWSE;
            }

            if (event == DOUBLE_CLICK ) {
                state = FLAG;
            }
            break;
        }

        case FLAG: {
            if (testControl(sleep_timer)) {
                outputState(false);
                outputPropDescr("FLAGS\0");
                prop_max = FLAGS_MAX;
                prop_value = PWR_FLAGS;
                prop_min = 0;
            }

            if (prop_value != c_flag) {
                PWR_FLAGS = (uint8_t) prop_value;
                itoa(PWR_FLAGS, BUFF, 2);
                oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
                c_flag = PWR_FLAGS;
                outputStatus(F("Decimal:"), PWR_FLAGS);
            }

            if (event == HOLD) {
                ctx->PERS.storeFLags();
                goToBrowse();
            }

            if (event == CLICK || sleep_timer.isTimeAfter(true)) {
                goToBrowse();
            }

            break;
        }

        case STORE: {
            firstRun();
            if (ctx->canAccessLocally()) {
                cmd->storeProps();
            } else {
                printCmd(cu.cmd_str.CMD_STORE_PROPS, NULL);
            }
            outputStatus(F("Storing... "), prop_value);
            delay(500);
            goToBrowse();
            break;
        }

        case SLEEP: {
            if (firstRun()) {
                sleep_timer.reset();
                dither = false;
                switchDisplay(false);
                c_prop_idx = -1; // invalidate cache;
            }


            if (sleep_timer.isTimeAfter(true)) {
                if (dither) {
                    state = SUSPEND;
                } else {
                    dither = true;
                    sleep_timer.reset();
                }

            }

            // Output Sleep Screen
            if (displayTiming.ping() && oled.getConnected()) {
                if (ctx->gaugeMode){
                    int16_t val = static_cast<int16_t>(*BANNER);
                    oled.outputLineGauge(1, static_cast<uint8_t>((val + 100) * 127 / 100), false);
                    oled.outputTextXY(DISPLAY_BASE + 2, 64, BANNER+2, true, dither);
                    oled.outputLineGauge(6, static_cast<uint8_t>((val + 100) * 127 / 100), true);
                } else {
                    oled.outputTextXY(DISPLAY_BASE + 2, 64, BANNER, true, dither);
                }
            }

            // Exit SLEEP state on event
            if (event == CLICK) {
                switchDisplay(true);
                state = BROWSE;
            };

            if (control_touched && ctx->props_default_idx >= 0 && canGoToEdit()) {
                control_touched = false;
                switchDisplay(true);
                prop_idx = (uint8_t) ctx->props_default_idx;
                goToEditProp(prop_idx);
            }

            break;
        }

        case SUSPEND: {
            if (firstRun()) {
                oled.displayOff();
            }

            if (event == CLICK || control_touched) {
                control_touched = false;
                oled.displayOn();
                state = SLEEP;
            };

            break;
        }
    }
    if (ctx->refreshState) {
        if(state == SUSPEND || state == SLEEP) {
            state = SLEEP;
        } else {
            outputState(true);
        }
        ctx->refreshState = false;
    }
    ctx->refreshProps = false;

}

bool Controller::testControl(TimingState &timer) const {
    bool fr = firstRun();
    if (fr || control_touched) {
        timer.reset();
        control_touched = false;
        if (fr) {
            outputState(true);
        }
    }
    return fr;
}

void Controller::goToBrowse() const {
    // invalidate cache
    c_prop_idx = -1;
    c_prop_value = -1;
    c_flag = 255;

    state = BROWSE;
}

bool Controller::firstRun() const {
    bool fr = oldState != state;
    oldState = state;
    return fr;
}

void Controller::goToEditProp(uint8_t i) const {

    c_prop_value = -1; // invalidate cache
    c_prop_idx = -1; // invalidate cache
    old_prop_value = prop_value; // for reminder in status
    state = EDIT_PROP;
}

bool inline Controller::canGoToEdit() {
    return !(ctx->passive && !ctx->connected && ctx->remote);
}


bool Controller::loadProperty(uint8_t idx) const {
    c_prop_idx = idx;
    if (ctx->canAccessLocally()) {
        flashStringHelperToChar(ctx->PROPERTIES[idx].desc, BUFF);
        copyProperty(ctx->PROPERTIES[idx], idx);
    } else {
        if (ctx->connected) {
            if (ctx->refreshProps) {
                copyProperty(ctx->remoteProperty, idx);
                requestForRefresh = false;
            } else {
                if (!requestForRefresh) {
                    printCmd(cu.cmd_str.CMD_GET_BIN_PROP, idxToChar(idx));
                    requestForRefresh = true;
                }
                return false;
            }
        } else {
            BUFF[0] = '\0';
        }
    }
    return true;
}

void Controller::copyProperty(Property &prop, uint8_t idx) const {
    long scale = prop.scale;
    prop_value = (prop.runtime / scale);
    prop_min = (prop.minv / scale);
    prop_max = (prop.maxv / scale);
    prop_measure = prop.measure;
}

void Controller::updateProperty(uint8_t idx) const {
    if (ctx->canAccessLocally()) {
        ctx->PROPERTIES[idx].runtime = prop_value * ctx->PROPERTIES[idx].scale;
    } else {
        if (ctx->connected) {
            sprintf(BUFF, "%i:%lu", idx, prop_value);
            printCmd(cu.cmd_str.CMD_SET_PROP, BUFF);
        }
    }
    c_prop_value = prop_value;
}

void Controller::switchDisplay(boolean inverse) const {
    oled.displayOff();
    inverse ? oled.setInverseDisplay() : oled.setNormalDisplay();
    oled.clearDisplay();
    oled.displayOn();
}

void Controller::outputPropDescr(char *_buff) {
    if (canGoToEdit()) {
        oled.setTextXY(DISPLAY_BASE, 0);
        padLine(_buff, 2, 0);
        oled.putString(_buff);
    }
}

void Controller::outputStatus(const __FlashStringHelper *txt, const long val) {
    flashStringHelperToChar(txt, BUFF);
    oled.setTextXY(DISPLAY_BOTTOM, 0);
    uint8_t prop_size = static_cast<uint8_t>(val > 0 ? log10((double) val) + 1 : 1);
    padLine(BUFF, 1, prop_size);
    oled.putString(BUFF);
    oled.setTextXY(DISPLAY_BOTTOM, (unsigned char) (LINE_SIZE - prop_size));
    oled.putNumber(val);
}

void Controller::padLine(char *_buff, uint8_t lines, uint8_t tail) {
    uint8_t t = LINE_SIZE * lines - tail;
    for (uint8_t i = strlen(_buff); i < t; i++) {
        _buff[i] = ' ';
    }
    _buff[t] = 0;
}

void Controller::outputPropVal(uint8_t measure_idx, int16_t _prop_val, bool brackets, bool measure) {
    const char *fmt =
            brackets && measure ? "[%i]%s" : (brackets & !measure ? "[%i]" : (!brackets && measure ? "%i%s"
                                                                                                   : "%i"));
    if (canGoToEdit()) {
        sprintf(BUFF, fmt, _prop_val, MEASURES[measure_idx]);
    } else {
        noInfoToBuff();
    }
    oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
}

#ifdef ENC1_PIN
#ifdef ENC2_PIN

ISR(PCVECT) {
    unsigned char input = encoder.process();
    if (input != NOTHING) {
        control_touched = true;
        if (!requestForRefresh) {
            switch (state) {

                case FLAG:
                case EDIT_PROP: {
                    input == DIR_CW ? DECR(prop_value, prop_min) : INCR(prop_value, prop_max);
                    break;
                }

                case BROWSE: {
                    input == DIR_CW ? DECR(prop_idx, 0) : INCR(prop_idx, props_idx_max);
                    break;
                }

                case STATES: {
                    input == DIR_CW ? DECR(state_idx, 0) : INCR(state_idx, state_idx_max);
                    break;
                }

            }
        }
    }
}

#endif
#endif

#endif // NO_CONTROLLER
