//
// Created by SHL on 20.03.2017.
//

#define INCR(val, max) val < max ? val++ : val
#define DECR(val, min) val > min ? val-- : val

#include <Rotary.h>
#include <ACROBOTIC_SSD1306.h>
#include "controller.h"

#ifndef NO_CONTROLLER

#define DISPLAY_BASE 1
#define DISPLAY_BOTTOM 7

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP, SENSORS, SUSPEND
} oldState = STORE, state = BROWSE;

static TimingState sleep_timer = TimingState(100000L);
static TimingState autoComplete_timer = TimingState(6000L);

static volatile bool control_touched = false;
static volatile uint8_t prop_idx = 0;
static volatile long prop_value;

static Property remoteProperty;
static char remotePropertyDesc[48];

static int8_t c_prop_idx = -1;
static long c_prop_value = -1;
static long old_prop_value;
static bool prop_changed;
static bool dither = false;

static volatile long prop_min;
static volatile long prop_max;

#if defined(ENC1_PIN) && defined(ENC2_PIN)
Rotary encoder = Rotary(ENC1_PIN, ENC2_PIN);
#endif

MultiClick encoderClick = MultiClick(ENC_BTN_PIN);
TimingState displayTiming = TimingState(1000L);
TimingState displaySearchTiming = TimingState(2000L);


Controller::Controller(Context &_ctx, Commands &_cmd) : cmd(&_cmd), ctx(&_ctx) {}

void Controller::begin() {
    initDisplay();
#if defined(ENC1_PIN) && defined(ENC2_PIN)
    initEncoderInterrupts();
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
    oled.setTextXY(0, 0);
    strlcpy(_cbuff, ctx-> passive ? "PASSIVE MODE" : ctx->id, 17);
    oled.putString(_cbuff);
}


void Controller::process() {

    McEvent event = encoderClick.checkButton();

    switch (state) {
        case EDIT_PROP: {

            testControl(autoComplete_timer);

            if (c_prop_value != prop_value || ctx->refreshProps) {
                updateProperty(prop_idx);
                outputPropDescr((uint8_t) prop_idx);
                outputPropVal(ctx->PROPERTIES[prop_idx], (int16_t) prop_value, true, true);
                outputStatus(F("Edit value:"), old_prop_value);
            }
            if (event == CLICK || autoComplete_timer.isTimeAfter(true)) {
                goToBrowse();
            };
            if (event == DOUBLE_CLICK) {
                prop_value = old_prop_value;
                updateProperty(prop_idx);
                goToBrowse();
            };
            if (event == HOLD) {
                state = STORE;
            };
            break;
        }
        case BROWSE: {

            testControl(sleep_timer);

            if (c_prop_idx != prop_idx || ctx->refreshProps) {
                loadProperty(prop_idx);

                outputPropDescr(prop_idx);
                outputPropVal(ctx->PROPERTIES[prop_idx], prop_value, false, true);
                outputStatus(F("Property:  "), prop_idx);
                prop_changed = false;
            }

            if (event == CLICK) {
                goToEditProp(prop_idx);
            };

            if (event == HOLD || sleep_timer.isTimeAfter(true)) {
                state = SLEEP;
            };

            break;
        }
        case STORE: {
            firstRun();
            cmd->storeProps();
            outputStatus(F("<Storing...>"), prop_value);
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
                oled.outputTextXY(DISPLAY_BASE + 2, 64, ctx->SENS.printDht(), true, dither);
            }

            // Exit SLEEP state on event
            if (event == CLICK ) {
                switchDisplay(true);
                state = BROWSE;
            };

            if (control_touched && ctx->defaultPropertyIdx >= 0){
                control_touched = false;
                switchDisplay(true);
                prop_idx = (uint8_t) ctx->defaultPropertyIdx;
                loadProperty(prop_idx);
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
    ctx->refreshProps = false;
    detectDisplay();
}

void Controller::testControl(TimingState &timer) const {
    if (firstRun() || control_touched){
                timer.reset();
                control_touched = false;
                outputTitle();
            }
}

void Controller::goToBrowse() const {
    c_prop_idx = -1; // invalidate cache;
    c_prop_value = -1; // invalidate cache;
    state = BROWSE;
}

bool Controller::firstRun() const {
    bool fr = oldState != state;
    oldState = state;
    return fr;
}

void Controller::goToEditProp(uint8_t i) const {
    c_prop_value = -1; // invalidate cache;
    c_prop_idx = -1; // invalidate cache;
    old_prop_value = prop_value; // for reminder in status
    state = EDIT_PROP;
}

void Controller::loadProperty(uint8_t idx) const {
    if (!passive) {
        cli();
        update(ctx->PROPERTIES[idx]);
        c_prop_idx = idx;
        sei();
    } else {
        Serial.print(cmd->cmd_str.CMD_GET_PROP);
        Serial.println(idx);
        delay(500);
        if (Serial.available()) {
            Serial.readBytesUntil('\n', remotePropertyDesc, 64);
            Serial.readBytes((uint8_t *)&remoteProperty, sizeof(Property));
            cli();
            update(remoteProperty);
            c_prop_idx = idx;
            sei();
        };
    }
}

void Controller::update(Property &prop) const {
    long scale = prop.scale;
    prop_value = (prop.runtime / scale);
    prop_min = (prop.minv / scale);
    prop_max = (prop.maxv / scale);
}

void Controller::updateProperty(uint8_t idx) const {
    if (!passive) {
        cli();
        ctx->PROPERTIES[idx].runtime = prop_value * ctx->PROPERTIES[idx].scale;
        c_prop_value = prop_value;
        sei();
    } else {
        Serial.print(cmd->cmd_str.CMD_SET_PROP);
        Serial.print(idx);
        Serial.print(':');
        Serial.println(prop_value);
    }
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

void Controller::outputPropDescr(uint8_t _idx) {
    if (oled.getConnected()) {
        oled.setTextXY(DISPLAY_BASE, 0);
        oled.putString(String(ctx->PROPERTIES[_idx].desc).c_str());
        oled.putString("      ");
    }
}

void Controller::outputStatus(const __FlashStringHelper *txt, const long val) {
    oled.setTextXY(DISPLAY_BOTTOM, 0);
    oled.putString(String(txt).c_str());
    oled.setTextXY(DISPLAY_BOTTOM, 12);
    oled.putNumber(val);
    oled.putString("  ");
}

void Controller::outputPropVal(Property &_prop, uint16_t _prop_val, bool brackets, bool _measure) {
    char str_text[12];
    const char *fmt =
            brackets && _measure ? "[%i]%s" : (brackets & !_measure ? "[%i]" : (!brackets && _measure ? "%i%s"
                                                                                                      : "%i"));
    sprintf(str_text, fmt, _prop_val, _prop.measure);
    oled.outputTextXY(DISPLAY_BASE + 2, 64, str_text, true, false);
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

#endif // NO_CONTROLLER
