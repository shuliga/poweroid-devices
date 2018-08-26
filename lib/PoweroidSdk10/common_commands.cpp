//
// Created by SHL on 26.08.2018.
//

#include "commons.h"
#include "common_commands.h"

Cu cu{
    cu.cmd_str = {"get_ver",
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
    }
};

const char *_cmd = " -> ";

String cmd;

int getValIndex() { return cmd.lastIndexOf(':') + 1; }

void printCmdResponse(const String &cmd, const char *suffix) {
    Serial.print(cmd);
    printCmd(_cmd, suffix);
}

void printCmd(const char *cmd, const char *suffix) {
    Serial.print(cmd);
    suffix != NULL ? Serial.println(suffix) : Serial.println();
    Serial.flush();
#ifdef DEBUG
    writeLog('I', ORIGIN, 100 + ctx->passive, cmd);
#endif
}

uint8_t getIndex() {
    uint8_t li = (uint8_t) cmd.lastIndexOf(':');
    return (uint8_t) cmd.substring((unsigned int) (cmd.lastIndexOf('_') + 1), li == -1 ? cmd.length() : li).toInt();
}

bool isCommand() {
    for (uint8_t i = 0; i < ARRAY_SIZE(cu.cmd_array); ++i) {
        if (cmd.startsWith(cu.cmd_array[i]) && cmd.indexOf(_cmd) < 0) {
            return true;
        }
    }
    return false;
}

bool castCommand(const char *prefix, const char *val) {
    if (cmd.startsWith(prefix)) {
        printCmdResponse(cmd, val);
        return true;
    }
    return false;
}


