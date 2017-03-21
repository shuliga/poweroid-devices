#include "PoweroidSdk10.h"
#include "bluetooth.h"
#include "commands.h"
#include "properties.h"

#define PREFIX(cmd) cmd + F(" -> ")

Commands::Commands(Context *_ctx): ctx(_ctx){
    persist = Persistence(SIGNATURE, ctx->RUNTIME, ctx->props_size);
}

void Commands::printProperty(int i) {
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
            for (int i = 0; i < ctx->props_size; i++) {
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
            for (int i = 0; i < ctx->props_size; i++) {
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_LOAD_PROPS)) {
            persist.loadProperties(ctx->RUNTIME);
            Serial.print(PREFIX(cmd));
            Serial.println(F("Properties loaded from EEPROM"));
            return;
        }
        if (cmd.startsWith(CMD_STORE_PROPS)) {
            storeProps();
            return;
        }
        if (cmd.startsWith(CMD_PREF_GET_PROP)) {
            int i = (int) cmd.substring(sizeof(CMD_PREF_GET_PROP)).toInt();
            if (i < ctx->props_size) {
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_SET_PROP)) {
            int i = (int) cmd.substring(sizeof(CMD_PREF_SET_PROP)).toInt();
            int idx = cmd.lastIndexOf(':') + 1;
            if (i < ctx->props_size && idx > 0) {
                long v = cmd.substring(idx).toInt();
                ctx->RUNTIME[i] = v * ctx->FACTORY[i].scale;
                Serial.print(PREFIX(cmd));
                printProperty(i);
            }
            return;
        }
        if (cmd.startsWith(CMD_GET_STATE_ALL)) {
            for (int i = 0; i < ctx->states_size; i++) {
                Serial.print(PREFIX(cmd));
                Serial.println(printState(states, i));
            }
            return;
        }
        if (cmd.startsWith(CMD_PREF_GET_STATE)) {
            int i = (int) cmd.substring(sizeof(CMD_PREF_GET_STATE)).toInt();
            if (i < ctx->props_size) {
                Serial.print(PREFIX(cmd));
                Serial.println(printState(states, i));
            }
            return;
        }
    }
}

void Commands::storeProps() {
    persist.storeProperties(ctx->RUNTIME);
    Serial.print(PREFIX(String(CMD_STORE_PROPS)));
    Serial.println(F("Properties stored to EEPROM"));
    return;
}
