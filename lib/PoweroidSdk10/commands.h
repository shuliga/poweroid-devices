#ifndef COMMANDS_H
#define COMMANDS_H

#include "commons.h"
#include "persistence.h"
#include "context.h"

typedef struct CommandsStr {
    const char* CMD_GET_VER;
    const char* CMD_GET_DHT;
    const char* CMD_GET_STATE;
    const char* CMD_DISARM_STATE;
    const char* CMD_GET_STATE_ALL;
    const char* CMD_GET_PROP;
    const char* CMD_GET_BIN_PROP;
    const char* CMD_SET_PROP;
    const char* CMD_GET_PROP_ALL;
    const char* CMD_GET_PROP_LEN;
    const char* CMD_LOAD_PROPS;
    const char* CMD_STORE_PROPS;
    const char* CMD_RESET_PROPS;
    const char* CMD_GET_SENSOR_ALL;
    const char* CMD_GET_RELAY_ALL;
    const char* ASK;
};

class Commands {
public:

    CommandsStr cmd_str;
    String cmd;

    Commands(Context &_ctx);

    void listen();

    void storeProps();

    void printChangedState(bool prev_state, bool state, uint8_t id);

private:

    Context *ctx;

    const char * printProperty(uint8_t i);

    int8_t getMappedFromVirtual(uint8_t i);

    uint8_t getIndex(const String &cmd) const;

    void printCmd(const String &cmd, const char *suffix) const;

    void printBinProperty(uint8_t i);

    bool execCommand(const char *prefix, const char *val);

    bool execCommandLoop(const char *prefix, uint8_t cnt, const char *(* val)(uint8_t i));

    const char *printSensor(uint8_t i);
};

#endif // COMMANDS_H
