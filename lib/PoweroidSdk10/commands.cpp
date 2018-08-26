
#include "global.h"
#include "commands.h"

static const char *ORIGIN = "CMD";

Commands::Commands(Context &_ctx) : ctx(&_ctx) {
    cmd_str = {"get_ver",
               "get_dht",
               "set_dht",
               "get_state_",
               "disarm_state_",
               "get_prop_",
               "get_bin_prop_",
               "set_bin_prop_",
               "set_prop_",
               "get_prop_len_bin",
               "load_props",
               "store_props",
               "reset_props",
               "get_relay",
               "mode ",
               "get_prop_all",
               "get_state_all",
               "get_sensor_all",
               "get_relay_all",
    };
}

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
    while (Serial.available()) {
        cmd = Serial.readStringUntil('\n');
        if (isCommand()) {
#ifdef DEBUG
            writeLog('I', ORIGIN, 200 + ctx->passive, cmd.c_str());
#endif
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    delay(5);

            castCommand(cmd_str.CMD_GET_VER, ctx->version);

            castCommand(cmd_str.CMD_GET_DHT, ctx->SENS.printDht());

            if (cmd.startsWith(cmd_str.MODE)) {
                ctx->peerReady = true;
                if (cmd.indexOf(MODE_ASK) > 0) {
                    printCmdResponse(cmd, ctx->passive ? MODE_CLIENT : MODE_SERVER);
                }
            }

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

            if (cmd.startsWith(cmd_str.CMD_SET_RELAY)) {
                Relays relays = ctx->RELAYS;
                int8_t i = ctx->passive ? relays.getMappedFromVirtual(getIndex()) : getIndex();
                if (i > 0 && i < relays.size()) {
                    ctx->refreshState = true;
                    relays.power(i, cmd.indexOf(REL_POWERED) > 0);
                    printCmdResponse(cmd, relays.printRelay(i));
                }
            }

//            if (cmd.startsWith(cmd_str.CMD_GET_PROP_ALL)) {
//                for (uint8_t i = 0; i < ctx->props_size; i++) {
//                    printCmdResponse(cmd, printProperty(i));
//                }
//            }

            if (castCommand(cmd_str.CMD_LOAD_PROPS, NULL)) {
                ctx->PERS.loadProperties(ctx->PROPERTIES);
                ctx->refreshProps = true;
            }

            if (castCommand(cmd_str.CMD_STORE_PROPS, NULL)) {
                storeProps();
            }

            if (cmd.startsWith(cmd_str.CMD_GET_PROP)) {
                uint8_t i = getIndex();
                if (i < ctx->props_size) {
                    printCmdResponse(cmd, printProperty(i));
                }
            }

            if (cmd.startsWith(cmd_str.CMD_SET_BIN_PROP)) {
                strcpy(BUFF, Serial.readStringUntil('\n').c_str());
                Serial.readBytes((uint8_t *) &ctx->remoteProperty, sizeof(Property));
                ctx->refreshProps = true;
            }

            if (cmd.startsWith(cmd_str.CMD_GET_BIN_PROP)) {
                uint8_t i = getIndex();
                if (i < ctx->props_size) {
                    printCmd(cmd_str.CMD_SET_BIN_PROP, idxToChar(i));
                    Serial.flush();
                    printBinProperty(i);
                    Serial.flush();
                }
            }

            if (cmd.startsWith(cmd_str.CMD_SET_PROP)) {
                uint8_t i = getIndex();
                int8_t idx = getValIndex();
                if (i < ctx->props_size && idx > 0) {
                    long v = cmd.substring((unsigned int) idx).toInt();
                    ctx->PROPERTIES[i].runtime = v * ctx->PROPERTIES[i].scale;
                    ctx->refreshProps = true;
                    printCmdResponse(cmd, printProperty(i));
                }
            }

//            if (cmd.startsWith(cmd_str.CMD_GET_STATE_ALL)) {
//                for (uint8_t i = 0; i < state_count; i++) {
//                    printCmdResponse(cmd, printState(i));
//                }
//            }
//
//            if (cmd.startsWith(cmd_str.CMD_GET_RELAY_ALL)) {
//                for (uint8_t i = 0; i < ctx->RELAYS.size(); i++) {
//                    printCmdResponse(cmd, ctx->RELAYS.printRelay(i));
//                }
//            }

            if (cmd.startsWith(cmd_str.CMD_GET_STATE)) {
                uint8_t i = getIndex();
                if (i < state_count) {
                    printCmdResponse(cmd, printState(i));
                }
            }

            if (cmd.startsWith(cmd_str.CMD_DISARM_STATE)) {
                uint8_t i = (uint8_t) getIndex();
                bool trigger = (bool) cmd.substring((unsigned int) getValIndex()).toInt();
                disarmState(i, trigger);
            }

            digitalWrite(LED_PIN, LOW);
        } else {
#ifdef DEBUG
            writeLog('I', ORIGIN, 210 + ctx->passive, cmd.c_str());
#endif
        }
    }
}

int Commands::getValIndex() const { return cmd.lastIndexOf(':') + 1; }

void Commands::printCmdResponse(const String &cmd, const char *suffix) const {
    Serial.print(cmd);
    printCmd(_cmd, suffix);
}

void Commands::printCmd(const char *cmd, const char *suffix) const {
    Serial.print(cmd);
    suffix != NULL ? Serial.println(suffix) : Serial.println();
    Serial.flush();
#ifdef DEBUG
    writeLog('I', ORIGIN, 100 + ctx->passive, cmd);
#endif
}

uint8_t Commands::getIndex() const {
    uint8_t li = (uint8_t) cmd.lastIndexOf(':');
    return (uint8_t) cmd.substring((unsigned int) (cmd.lastIndexOf('_') + 1), li == -1 ? cmd.length() : li).toInt();
}

void Commands::storeProps() {
    ctx->PERS.storeProperties(ctx->PROPERTIES);
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
        printCmd(cmd_str.MODE, ctx->passive ? MODE_SERVER : MODE_CLIENT);
        connected = ctx->peerReady;
        ctx->peerReady = false;
        connection_check.reset();
#ifdef DEBUG
        if (ctx->passive && !connected) {
            writeLog('W', ORIGIN, 410);
        }
#endif
    }
    return connected;
}

bool inline Commands::isCommand() {
    for (uint8_t i = 0; i < ARRAY_SIZE(cmd_array); ++i) {
        if (cmd.startsWith(cmd_array[i]) && cmd.indexOf(_cmd) < 0) {
            return true;
        }
    }
    return false;
}

void Commands::disarmState(uint8_t i, bool disarm) {
    ctx->PERS.storeState(i, disarm);
    printCmdResponse(cmd, printState(i));
}

