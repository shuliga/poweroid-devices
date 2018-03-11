
#include "global.h"
#include "commands.h"

#define PREFIX(cmd) cmd + " -> "

Commands::Commands(Context &_ctx) : ctx(&_ctx) {
    cmd_str = {"get_ver",
               "get_dht",
               "get_state_",
               "disarm_state_",
               "get_state_all",
               "get_prop_",
               "get_bin_prop_",
               "set_prop_",
               "get_prop_all",
               "get_prop_len",
               "load_props",
               "store_props",
               "reset_props",
               "get_sensor_all",
               "get_relay_all",
               "ask"
    };
}

char *Commands::printProperty(uint8_t i) {
    char _desc[64];
    flashStringHelperToChar(ctx->PROPERTIES[i].desc, _desc);
    sprintf(BUFF, "[%i] %s : %i", i, _desc,
            (int) (ctx->PROPERTIES[i].runtime / ctx->PROPERTIES[i].scale));
    return BUFF;
}

void Commands::printBinProperty(uint8_t i) {
    Serial.println(ctx->PROPERTIES[i].desc);
    for (uint8_t j = 0; j < sizeof(Property); j++) {
        Serial.write(*((uint8_t*) &ctx->PROPERTIES[i] + j));
    }
}

void Commands::listen() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
#ifdef DEBUG
        writeLog('I', "CMD", 100, cmd.c_str());
#endif
        if (ctx->passive) {
#ifdef SSERIAL
            SSerial.println(cmd);
#endif
            if (cmd.startsWith(REL_PREFIX)) {
                uint8_t ri = (uint8_t) cmd.substring((unsigned int) (cmd.indexOf('[') + 1),
                                                     (unsigned int) cmd.indexOf(']')).toInt();
                int8_t i = getMappedFromVirtual(ri);
                if (i >= 0) {
                    ctx->RELAYS.power((uint8_t) i, cmd.indexOf(REL_POWERED) > 0, false);
                }
                return;
            }
        }

        if (cmd.startsWith(cmd_str.ASK)) {
            printCmd(cmd, ctx->passive ? ASK_CLIENT : ASK_SERVER);
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_VER)) {
            printCmd(cmd, ctx->version);
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_DHT)) {
            printCmd(cmd, ctx->SENS.printDht());
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_SENSOR_ALL)) {
            for (uint8_t i = 0; i < ctx->SENS.size(); i++) {
                printCmd(cmd, ctx->SENS.printSensor(i));
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_RESET_PROPS)) {
            printCmd(cmd, NULL);
            for (uint8_t i = 0; i < ctx->props_size; i++) {
                ctx->PROPERTIES[i].runtime = ctx->PROPERTIES[i].val;
            }
            ctx->refreshProps = true;
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_PROP_LEN)) {
            char num[5];
            printCmd(cmd, itoa(ctx->props_size, num, 10));
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_PROP_ALL)) {
            for (uint8_t i = 0; i < ctx->props_size; i++) {
                printCmd(cmd, printProperty(i));
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_LOAD_PROPS)) {
            printCmd(cmd, NULL);
            ctx->PERS.loadProperties(ctx->PROPERTIES);
            ctx->refreshProps = true;
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_STORE_PROPS)) {
            printCmd(cmd_str.CMD_STORE_PROPS, NULL);
            storeProps();
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_PROP)) {
            uint8_t i = getIndex(cmd);
            if (i < ctx->props_size) {
                printCmd(cmd, printProperty(i));
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_BIN_PROP)) {
            uint8_t i = getIndex(cmd);
            if (i < ctx->props_size) {
                printBinProperty(i);
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_SET_PROP)) {
            uint8_t i = getIndex(cmd);
            int8_t idx = cmd.lastIndexOf(':') + 1;
            if (i < ctx->props_size && idx > 0) {
                long v = cmd.substring((unsigned int) idx).toInt();
                ctx->PROPERTIES[i].runtime = v * ctx->PROPERTIES[i].scale;
                ctx->refreshProps = true;
                printCmd(cmd, printProperty(i));
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_STATE_ALL)) {
            for (uint8_t i = 0; i < ctx->states_size; i++) {
                printCmd(cmd, ctx->printState(i, BUFF));
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_RELAY_ALL)) {
            for (uint8_t i = 0; i < ctx->RELAYS.size(); i++) {
                printCmd(cmd, NULL);
                ctx->RELAYS.printRelay(i);
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_GET_STATE)) {
            uint8_t i = getIndex(cmd);
            if (i < ctx->states_size) {
                printCmd(cmd, ctx->printState(i, BUFF));
            }
            return;
        }

        if (cmd.startsWith(cmd_str.CMD_DISARM_STATE)) {
            uint8_t i = (uint8_t) getIndex(cmd);
            bool trigger = (bool) cmd.substring((unsigned int) (cmd.lastIndexOf(':') + 1)).toInt();
            if (i < ctx->states_size) {
                ctx->disarmState(i, trigger);
                ctx->PERS.storeState(i, trigger);
                printCmd(cmd, ctx->printState(i, BUFF));
            }
            return;
        }
    }

}

void Commands::printCmd(const String &cmd, const char *suffix) const {
    Serial.print(PREFIX(cmd));
    if (suffix != NULL) Serial.println(suffix); else Serial.println();
}

uint8_t Commands::getIndex(const String &cmd) const {
    uint8_t li = (uint8_t) cmd.lastIndexOf(':');
    return (uint8_t) cmd.substring((unsigned int) (cmd.lastIndexOf('_') + 1), li == -1 ? cmd.length() : li).toInt();
}

void Commands::storeProps() {
    ctx->PERS.storeProperties(ctx->PROPERTIES);
    return;
}

int8_t Commands::getMappedFromVirtual(uint8_t i) {
    for (uint8_t idx = 0; idx < VIRTUAL_RELAYS; idx++) {
        int8_t mappedRelay = ctx->RELAYS.mappings[idx];
        if (mappedRelay == i) return idx;
    }
    return -1;
}
