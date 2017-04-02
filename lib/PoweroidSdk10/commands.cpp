#include "context.h"
#include "properties.h"
#include "commands.h"
#include "bluetooth.h"
#include "PoweroidSdk10.h"

#define PREFIX(cmd) cmd + F(" -> ")

Commands::Commands(Context &_ctx): ctx(&_ctx){
    persist = new Persistence(ctx->signature, ctx->RUNTIME, ctx->props_size);
}

void Commands::printProperty(uint8_t i) {
    Serial.print("[");
    Serial.print(i);
    Serial.print("] ");
    Serial.print(ctx->FACTORY[i].desc);
    Serial.print(" : ");
    Serial.println(ctx->RUNTIME[i] / ctx->FACTORY[i].scale);
}

void Commands::listen() {
    if (Serial.available() > 0) {
        String cmd = Serial.readString();
        cmd.replace("\n", "");

        if (cmd.startsWith(CMD_GET_VER)) {
            Serial.print(PREFIX(cmd));
            Serial.print(ctx->version);
            Serial.print("-");
            Serial.print(BOARD_VERSION);
            return;
        }

        if (cmd.startsWith(F(CMD_GET_DHT))) {
            Serial.print(PREFIX(cmd));
            Serial.println(ctx->SENS->printDht());
            return;
        }

        if (cmd.startsWith(CMD_GET_SENSOR_ALL)) {
            for (uint8_t i = 0; i < ctx->SENS->size(); i++) {
                Serial.print(PREFIX(cmd));
                Serial.println(ctx->SENS->printSensor(i));
            }
            return;
        }

        if (cmd.startsWith(CMD_RESET_PROPS)) {
            for (uint8_t i = 0; i < ctx->props_size; i++) {
                ctx->RUNTIME[i] = ctx->FACTORY[i].val;
            }
            Serial.print(PREFIX(cmd));
            Serial.println(F("Properties reset to factory settings"));
            return;
        }

        if (cmd.startsWith(CMD_GET_PROP_LEN)) {
            Serial.print(PREFIX(cmd));
            Serial.println(ctx->props_size);
            return;
        }
        if (cmd.startsWith(CMD_GET_PROP_ALL)) {
            for (uint8_t i = 0; i < ctx->props_size; i++) {
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_LOAD_PROPS)) {
            persist->loadProperties(ctx->RUNTIME);
            Serial.print(PREFIX(cmd));
            Serial.println(F("Properties loaded from EEPROM"));
            return;
        }
        if (cmd.startsWith(CMD_STORE_PROPS)) {
            storeProps();
            return;
        }
        if (cmd.startsWith(CMD_PREF_GET_PROP)) {
            uint8_t i = (uint8_t) cmd.substring(sizeof(CMD_PREF_GET_PROP) - 1).toInt();
            if (i < ctx->props_size) {
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_SET_PROP)) {
            uint8_t i = (uint8_t ) cmd.substring(sizeof(CMD_PREF_SET_PROP) - 1).toInt();
            int8_t idx = cmd.lastIndexOf(':') + 1;
            if (i < ctx->props_size && idx > 0) {
                long v = cmd.substring(idx).toInt();
                ctx->RUNTIME[i] = v * ctx->FACTORY[i].scale;
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_GET_STATE_ALL)) {
            for (uint8_t i = 0; i < ctx->states_size; i++) {
                Serial.print(PREFIX(cmd));
                ctx->printState(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_GET_STATE)) {
            uint8_t i = (uint8_t) cmd.substring(sizeof(CMD_PREF_GET_STATE)).toInt();
            if (i < ctx->states_size) {
                Serial.print(PREFIX(cmd));
                ctx->printState(i);
            }
            return;
        }

        if (cmd.startsWith(CMD_PREF_DISARM_STATE)) {
            uint8_t i = (uint8_t) cmd.substring(sizeof(CMD_PREF_DISARM_STATE) - 1, sizeof(CMD_PREF_DISARM_STATE)).toInt();
            bool trigger = (bool) cmd.substring((cmd.lastIndexOf(':') + 1)).toInt();
            if (i < ctx->states_size) {
                ctx->disarmState(i, trigger);
            }
            return;
        }
    }
}

void Commands::storeProps() {
    persist->storeProperties(ctx->RUNTIME);
    Serial.print(PREFIX(String(CMD_STORE_PROPS)));
    Serial.println(F("Properties stored to EEPROM"));
    return;
}
