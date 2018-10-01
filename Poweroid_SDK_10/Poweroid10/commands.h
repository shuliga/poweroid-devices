//
// Created by SHL on 26.08.2018.
//

#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>

typedef struct CommandsStr {
    const char* CMD_GET_VER;
    const char* CMD_GET_DHT;
    const char* CMD_SET_DHT;
    const char* CMD_GET_STATE;
    const char* CMD_DISARM_STATE;
    const char* CMD_GET_PROP;
    const char* CMD_GET_BIN_PROP;
    const char* CMD_SET_BIN_PROP;
    const char* CMD_SET_PROP;
    const char* CMD_LOAD_PROPS;
    const char* CMD_STORE_PROPS;
    const char* CMD_RESET_PROPS;
    const char* CMD_SET_RELAY;
    const char* MODE;
    const char* CMD_GET_PROP_ALL;
#ifndef SAVE_RAM
    const char* CMD_GET_STATE_ALL;
    const char* CMD_GET_SENSOR_ALL;
    const char* CMD_GET_RELAY_ALL;
#endif
};

extern union Cu{
    CommandsStr cmd_str;
    char * cmd_array[19];
} cu;

extern const char *cmd_arrow;

extern String cmd;

void printCmdResponse(const String &cmd, const char *suffix);

void printCmd(const char *cmd, const char *suffix);

uint8_t getIndex();

int getValIndex();

bool castCommand(const char *prefix, const char *val);

bool isCommand();

#endif //COMMANDS_H
