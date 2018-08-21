
#include "global.h"
#include "commands.h"

static const char *ORIGIN = "CMD";

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
               "get_prop_len_bin",
               "load_props",
               "store_props",
               "reset_props",
               "get_sensor_all",
               "get_relay_all",
               "ask"
    };
}

typedef const char *(Commands::*Index_fn_ptr)(uint8_t i);

const char *_cmd = " -> ";
const char *STATE_FORMAT_BUFF = {"[%i] State %s: %s"};

static TimingState connection_check(CONNECTION_CHECK);

const char *Commands::printProperty(uint8_t i) {
    char _desc[64];
    flashStringHelperToChar(ctx->PROPERTIES[i].desc, _desc);
    sprintf(BUFF, "[%i] %s : %i", i, _desc,
            (int) (ctx->PROPERTIES[i].runtime / ctx->PROPERTIES[i].scale));
    return BUFF;
}

void Commands::printBinProperty(uint8_t i) {
    Serial.println(ctx->PROPERTIES[i].desc);
    for (uint8_t j = 0; j < sizeof(Property); j++) {
        Serial.write(*((uint8_t *) &ctx->PROPERTIES[i] + j));
    }
}

void Commands::listen() {
    //
    // processStatus
    //
    while (Serial.available()) {
        cmd = Serial.readStringUntil('\n');
        if (isCommand()) break;
#ifdef DEBUG
        writeLog('I', "STS", 100 + ctx->passive, cmd.c_str());
#endif
        if (ctx->passive && cmd.indexOf(REL_PREFIX) >= 0) {
            uint8_t ri = (uint8_t) cmd.substring((unsigned int) (cmd.indexOf('[') + 1),
                                                 (unsigned int) cmd.indexOf(']')).toInt();
            int8_t i = getMappedFromVirtual(ri);
            if (i >= 0) {
                ctx->refreshState = true;
                ctx->RELAYS.power((uint8_t) i, cmd.indexOf(REL_POWERED) > 0);
            }
        }
    }

    //
    // processCommand
    //
    if (isCommand()) {
#ifdef DEBUG
        writeLog('I', ORIGIN, 100 + ctx->passive, cmd.c_str());
#endif
        castCommand(cmd_str.ASK, ctx->passive ? ASK_CLIENT : ASK_SERVER);

        castCommand(cmd_str.CMD_GET_VER, ctx->version);

        castCommand(cmd_str.CMD_GET_DHT, ctx->SENS.printDht());

        if (cmd.startsWith(cmd_str.CMD_GET_SENSOR_ALL)) {
            for (uint8_t i = 0; i < ctx->SENS.size(); i++) {
                printCmdResponse(cmd, ctx->SENS.printSensor(i));
            }
        }

        if (cmd.startsWith(cmd_str.CMD_RESET_PROPS)) {
            printCmdResponse(cmd, NULL);
            for (uint8_t i = 0; i < ctx->props_size; i++) {
                ctx->PROPERTIES[i].runtime = ctx->PROPERTIES[i].val;
            }
            ctx->refreshProps = true;
        }

        char len[2] = {ctx->props_size, 0};
        castCommand(cmd_str.CMD_GET_PROP_LEN_BIN, len);

        if (cmd.startsWith(cmd_str.CMD_GET_PROP_ALL)) {
            for (uint8_t i = 0; i < ctx->props_size; i++) {
                printCmdResponse(cmd, printProperty(i));
            }
        }

        if (castCommand(cmd_str.CMD_LOAD_PROPS, NULL)) {
            ctx->PERS.loadProperties(ctx->PROPERTIES);
            ctx->refreshProps = true;
        }

        if (castCommand(cmd_str.CMD_STORE_PROPS, NULL)) {
            storeProps();
        }

        if (cmd.startsWith(cmd_str.CMD_GET_PROP)) {
            uint8_t i = getIndex(cmd);
            if (i < ctx->props_size) {
                printCmdResponse(cmd, printProperty(i));
            }
        }

        if (cmd.startsWith(cmd_str.CMD_GET_BIN_PROP)) {
            uint8_t i = getIndex(cmd);
            if (i < ctx->props_size) {
                printBinProperty(i);
            }
        }

        if (cmd.startsWith(cmd_str.CMD_SET_PROP)) {
            uint8_t i = getIndex(cmd);
            int8_t idx = cmd.lastIndexOf(':') + 1;
            if (i < ctx->props_size && idx > 0) {
                long v = cmd.substring((unsigned int) idx).toInt();
                ctx->PROPERTIES[i].runtime = v * ctx->PROPERTIES[i].scale;
                ctx->refreshProps = true;
                printCmdResponse(cmd, printProperty(i));
            }
        }

        if (cmd.startsWith(cmd_str.CMD_GET_STATE_ALL)) {
            for (uint8_t i = 0; i < state_count; i++) {
                printCmdResponse(cmd, printState(i));
            }
        }

        if (cmd.startsWith(cmd_str.CMD_GET_RELAY_ALL)) {
            for (uint8_t i = 0; i < ctx->RELAYS.size(); i++) {
                printCmdResponse(cmd, NULL);
                ctx->RELAYS.printRelay(i);
            }
        }

        if (cmd.startsWith(cmd_str.CMD_GET_STATE)) {
            uint8_t i = getIndex(cmd);
            if (i < state_count) {
                printCmdResponse(cmd, printState(i));
            }
        }

        if (cmd.startsWith(cmd_str.CMD_DISARM_STATE)) {
            uint8_t i = (uint8_t) getIndex(cmd);
            bool trigger = (bool) cmd.substring((unsigned int) (cmd.lastIndexOf(':') + 1)).toInt();
            if (i < state_count) {
                disarmState(i, trigger);
                ctx->PERS.storeState(i, trigger);
                printCmdResponse(cmd, printState(i));
            }
        }

    }
    cmd = "";
}


bool Commands::isResponse(const char *c) const { return strstr(c, "->") != NULL; }

void Commands::printCmdResponse(const String &cmd, const char *suffix) const {
    Serial.print(cmd);
    printCmd(_cmd, suffix);
}

void Commands::printCmd(const char *cmd, const char *suffix) const {
    Serial.print(cmd);
    suffix != NULL ? Serial.println(suffix) : Serial.println();
}


void Commands::executeCmd(const char *_cmd, const char *suffix) {
    listen();
    printCmd(_cmd, suffix);
    Serial.flush();
    consumeSerialToBuff();
#ifdef DEBUG
    writeLog('I', ORIGIN, 200 + ctx->passive, _cmd);
    writeLog('I', ORIGIN, 210 + ctx->passive, BUFF);
#endif
}

void Commands::consumeSerialToBuff() const { BUFF[Serial.readBytesUntil('\n', BUFF, BUFF_SIZE)] = 0; }


uint8_t Commands::getIndex(const String &cmd) const {
    uint8_t li = (uint8_t) cmd.lastIndexOf(':');
    return (uint8_t) cmd.substring((unsigned int) (cmd.lastIndexOf('_') + 1), li == -1 ? cmd.length() : li).toInt();
}

void Commands::storeProps() {
    ctx->PERS.storeProperties(ctx->PROPERTIES);
}

int8_t Commands::getMappedFromVirtual(uint8_t i) {
    for (uint8_t idx = 0; idx < VIRTUAL_RELAYS; idx++) {
        int8_t mappedRelay = ctx->RELAYS.mappings[idx];
        if (mappedRelay == i) return idx;
    }
    return -1;
}

const char *printState(uint8_t i) {
    sprintf(BUFF, STATE_FORMAT_BUFF, i, getState(i)->name, getState(i)->state);
    return BUFF;
}

void Commands::printChangedState(bool prev_state, bool state, uint8_t id) {
    if (prev_state != state) {
        printState(id);
    }
}

bool Commands::castCommand(const char *prefix, const char *val) {
    if (cmd.startsWith(prefix)) {
        printCmdResponse(cmd, val);
        return true;
    }
    return false;
}

bool Commands::isConnected() {
    if (connection_check.isTimeAfter(true)) {
        connected = ctx->passive ? checkPeerType(ASK_SERVER) : checkPeerType(ASK_CLIENT);
        connection_check.reset();
#ifdef DEBUG
        if (ctx->passive && !connected) {
            writeLog('W', ORIGIN, 410);
        }
#endif
    }
    return connected;
}

bool Commands::checkPeerType(const char *type) {
    executeCmd(cmd_str.ASK, NULL);
    return strstr(BUFF, type) != NULL;
}

bool Commands::isCommand() {
    for (uint8_t i = 0; i < ARRAY_SIZE(cmd_array); ++i) {
        if (cmd.startsWith(cmd_array[i])) {
            return true;
        }
    }
    return false;
}

