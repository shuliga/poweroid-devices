//
// Created by SHL on 20.03.2017.
//

#define INCR(val, max) val < max ? val++ : val
#define DECR(val, min) val > min ? val-- : val

#include "global.h"
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

static int8_t c_prop_idx = -1;
static long c_prop_value = -1;
static long old_prop_value;
static bool dither = false;

static volatile long prop_min;
static volatile long prop_max;
static volatile uint8_t prop_measure;

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
    oled.setTextXY(0, 0);
    clearBuff(ONE_LINE);
    strlcpy(BUFF, ctx->passive ? "PASSIVE MODE    " : ctx->id, ONE_LINE);
    oled.putString(BUFF);
}


void Controller::process() {

    McEvent event = encoderClick.checkButton();

    switch (state) {
        case EDIT_PROP: {

            if (testControl(autoComplete_timer)) {
                loadProperty(prop_idx);
                outputPropDescr(BUFF);
                outputStatus(F("Edit value:"), old_prop_value);
            }

            if (c_prop_value != prop_value || ctx->refreshProps) {
                updateProperty(prop_idx);
                outputPropVal(prop_measure, (int16_t) prop_value, true, true);
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
                outputPropDescr(BUFF);
                outputPropVal(prop_measure, (int16_t) prop_value, false, true);
                outputStatus(F("Property:"), prop_idx + 1);
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
            if (event == CLICK) {
                switchDisplay(true);
                state = BROWSE;
            };

            if (control_touched && ctx->defaultPropertyIdx >= 0) {
                control_touched = false;
                switchDisplay(true);
                prop_idx = (uint8_t) ctx->defaultPropertyIdx;
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

bool Controller::testControl(TimingState &timer) const {
    bool fr = firstRun();
    if (fr || control_touched) {
        timer.reset();
        control_touched = false;
        outputTitle();
    }
    return fr;
}

void Controller::goToBrowse() const {
    c_prop_idx = -1; // invalidate cache
    c_prop_value = -1; // invalidate cache
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

void Controller::loadProperty(uint8_t idx) const {
    clearBuff(64);
    if (!ctx->passive) {
        flashStringHelperToChar(ctx->PROPERTIES[idx].desc, BUFF);
        copyProperty(ctx->PROPERTIES[idx], idx);
    } else {
        Serial.print(cmd->cmd_str.CMD_GET_BIN_PROP);
        Serial.println(idx);
        Serial.readBytesUntil('\n', BUFF, BUFF_SIZE);
        Serial.readBytes((uint8_t *) &remoteProperty, sizeof(Property));
        copyProperty(remoteProperty, idx);
    }
}

void Controller::copyProperty(Property &prop, uint8_t idx) const {
    cli();
    update(prop);
    c_prop_idx = idx;
    sei();
}

void Controller::clearBuff(int i)const {
    memset(BUFF, ' ', BUFF_SIZE);
    BUFF[i] = 0;
}


void Controller::update(Property &prop) const {
    long scale = prop.scale;
    prop_value = (prop.runtime / scale);
    prop_min = (prop.minv / scale);
    prop_max = (prop.maxv / scale);
    prop_measure = prop.measure;
}

void Controller::updateProperty(uint8_t idx) const {
    if (!ctx->passive) {
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

void Controller::outputPropDescr(char *_buff) {
    if (oled.getConnected()) {
        oled.setTextXY(DISPLAY_BASE, 0);
        oled.putString(_buff);
    }
}

void Controller::outputStatus(const __FlashStringHelper *txt, const long val) {
    clearBuff(32);
    flashStringHelperToChar(txt, BUFF);
    oled.setTextXY(DISPLAY_BOTTOM, 0);
    oled.putString(BUFF);
    oled.setTextXY(DISPLAY_BOTTOM, (unsigned char) (16 - PROP_SIZE));
    for(uint8_t i = 0; i < (PROP_SIZE - getNumberOfDigits(val)); i++){
        oled.putString(" ");
    }
    oled.putNumber(val);
}

uint8_t Controller::getNumberOfDigits(long i)
{
    return i > 0 ? (uint8_t) log10 ((double) i) + 1 : 1;
}

void Controller::outputPropVal(uint8_t measure_idx, int16_t _prop_val, bool brackets, bool measure) {
    char str_text[12];
    const char *fmt =
            brackets && measure ? "[%i]%s" : (brackets & !measure ? "[%i]" : (!brackets && measure ? "%i%s"
                                                                                                   : "%i"));
    sprintf(str_text, fmt, _prop_val, MEASURES[measure_idx]);
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
