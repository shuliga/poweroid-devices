#include "PoweroidSdk10.h"
#include "bluetooth.h"
#include "commands.h"

#include <Arduino.h>
#include <Rotary.h>
#include <MultiClick.h>
#include <ACROBOTIC_SSD1306.h>

#define INCR(val, max) val < max ? val++ : val
#define DECR(val, min) val > min ? val-- : val

#define BUTTON_PIN 7

Rotary rotaryEnc = Rotary();
MultiClick rotaryClick(BUTTON_PIN);

static volatile int props_ixd_max = 0;

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP
} oldState = STORE, state = BROWSE;


static volatile int prop_idx = 0;
static int c_prop_idx = -1;

static volatile long prop_value;
static long c_prop_value = -1;
static long old_prop_value;
static volatile long prop_min;
static volatile long prop_max;

#define PREFIX(cmd) cmd + F(" -> ")

Commands::Commands(Property *p, long *p_r, int ps, String *s, int ss, char *_id) : props_factory(p), props_runtime(p_r),
                                                                                   props_size(ps), states(s),
                                                                                   states_size(ss), id(_id) {
    persist = Persistence(SIGNATURE, props_runtime, props_size);
#ifdef  BT
    bt = Bt(id);
#endif
    initDisplay();
    initRotaryInterrupts();
}

void Commands::initRotaryInterrupts() {
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
    sei();
    props_ixd_max = props_size - 1;
}

void Commands::initDisplay() {
    if (oled.checkAndInit()) {
        oled.clearDisplay();
        oled.setBrightness(0);
        oled.setTextXY(0, 1);
        oled.putString(id);
    };
}

void Commands::printProperty(int i) {
    Serial.print("[");
    Serial.print(i);
    Serial.print("] ");
    Serial.print(props_factory[i].desc);
    Serial.print(" : ");
    Serial.println(props_runtime[i] / props_factory[i].scale);
}

void Commands::listen() {
    if (Serial.available() > 0) {
        String cmd = Serial.readString();
        cmd.replace("\n", "");

        if (cmd.startsWith(F("bt_status"))) {
            bt.getResult();
            return;
        }

        if (cmd.startsWith(F("get_ver"))) {
            Serial.print(PREFIX(cmd));
            Serial.println(VERSION);
            return;
        }

        if (cmd.startsWith(CMD_GET_SENSOR_ALL)) {
            for (int i = 0; i < ARRAY_SIZE(installed); i++) {
                Serial.print(PREFIX(cmd));
                Serial.println(printSensor(installed, i));
            }
            return;
        }

        if (cmd.startsWith(CMD_RESET_PROPS)) {
            for (int i = 0; i < props_size; i++) {
                props_runtime[i] = props_factory[i].val;
            }
            Serial.print(PREFIX(cmd));
            Serial.println(F("Properties reset to factory settings"));
            return;
        }

        if (cmd.startsWith(CMD_GET_PROP_LEN)) {
            Serial.print(PREFIX(cmd));
            Serial.println(props_size);
            return;
        }
        if (cmd.startsWith(CMD_GET_PROP_ALL)) {
            for (int i = 0; i < props_size; i++) {
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_LOAD_PROPS)) {
            persist.loadProperties(props_runtime);
            Serial.print(PREFIX(cmd));
            Serial.println(F("Properties loaded from EEPROM"));
            return;
        }
        if (cmd.startsWith(CMD_PREF_STORE_PROPS)) {
            storeProps(cmd);
            return;
        }
        if (cmd.startsWith(CMD_PREF_GET_PROP)) {
            int i = (int) cmd.substring(sizeof(CMD_PREF_GET_PROP)).toInt();
            if (i < props_size) {
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_SET_PROP)) {
            int i = (int) cmd.substring(sizeof(CMD_PREF_SET_PROP)).toInt();
            int idx = cmd.lastIndexOf(':') + 1;
            if (i < props_size && idx > 0) {
                long v = cmd.substring(idx).toInt();
                props_runtime[i] = v * props_factory[i].scale;
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_GET_STATE_ALL)) {
            for (int i = 0; i < states_size; i++) {
                Serial.print(PREFIX(cmd));
                Serial.println(printState(states, i));
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_GET_STATE)) {
            int i = (int) cmd.substring(sizeof(CMD_PREF_GET_STATE)).toInt();
            if (i < props_size) {
                Serial.print(PREFIX(cmd));
                Serial.println(printState(states, i));
            }
            return;
        }
    }
    processRotary();
}

void Commands::processRotary() {

    McEvent event = rotaryClick.checkButton();

    switch (state) {
        case EDIT_PROP: {
            if (c_prop_value != prop_value) {
                cli();
                props_runtime[prop_idx] = prop_value * props_factory[prop_idx].scale;
                c_prop_value = prop_value;
                sei();
                printPropDescr((uint8_t) prop_idx);
                printPropVal((int) prop_value, true);
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
                prop_value = (props_runtime[prop_idx] / props_factory[prop_idx].scale);
                prop_min = (props_factory[prop_idx].minv / props_factory[prop_idx].scale);
                prop_max = (props_factory[prop_idx].maxv / props_factory[prop_idx].scale);
                c_prop_idx = prop_idx;
                sei();

                printPropDescr(prop_idx);
                printPropVal(prop_value, false);
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
            storeProps(CMD_PREF_STORE_PROPS);
            c_prop_value = -1; // invalidate cache;
            c_prop_idx = -1; // invalidate cache;
            oldState = state;
            state = BROWSE;
            break;
        }
        case SLEEP: {
            if (oldState != state){
                oled.setNormalDisplay();
                delay(500);
                oled.displayOff();
            }
            if (event == CLICK) {
                oled.setInverseDisplay();
                oled.displayOn();

                oldState = state;
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

void Commands::printPropDescr(uint8_t _idx) {
    if (oled.isConnected()) {
        oled.setTextXY(1, 0);
        oled.putString(props_factory[_idx].desc);
        oled.putString("      ");
    }
}

void Commands::outputStatus(const __FlashStringHelper *txt, const long val) {
    oled.setTextXY(7, 0);
    oled.putString(txt);
    oled.setTextXY(7, 12);
    oled.putNumber(val);
    oled.putString("  ");
}

void Commands::printPropVal(int _prop_val, bool brackets) {
    char str_text[10];
    if (brackets){
        sprintf(str_text, "[%i]", _prop_val);
    } else{
        sprintf(str_text, "%i", _prop_val);
    }
    oled.outputTextXY(3, 64, str_text, true);
}


void Commands::storeProps(String cmd) {
    persist.storeProperties(props_runtime);
    Serial.print(PREFIX(cmd));
    Serial.println(F("Properties stored to EEPROM"));
    return;
}


ISR(PCINT2_vect) {
    unsigned char result = rotaryEnc.process();
    if (result != NOTHING) {
        switch (state) {
            case EDIT_PROP: {
                result == DIR_CW ? DECR(prop_value, prop_min) : INCR(prop_value, prop_max);
                break;
            }
            case BROWSE: {
                result == DIR_CW ? DECR(prop_idx, 0) : INCR(prop_idx, props_ixd_max);
                break;
            }
            case STORE:
                break;
        }
    }
}