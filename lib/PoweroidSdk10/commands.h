#ifndef COMMANDS_H
#define COMMANDS_H

#define CMD_GET_VER "get_ver"
#define CMD_GET_DHT "get_dht"
#define CMD_PREF_GET_STATE "get_state_"
#define CMD_PREF_DISARM_STATE "disarm_state_"
#define CMD_GET_STATE_ALL "get_state_all"
#define CMD_PREF_GET_PROP "get_prop_"
#define CMD_PREF_SET_PROP "set_prop_"
#define CMD_GET_PROP_ALL "get_prop_all"
#define CMD_GET_PROP_LEN "get_prop_len"
#define CMD_LOAD_PROPS "load_props"
#define CMD_STORE_PROPS "store_props"
#define CMD_RESET_PROPS "reset_props"
#define CMD_GET_SENSOR_ALL "get_sensor_all"
#define CMD_GET_RELAY_ALL "get_relay_all"

#include "commons.h"
#include "context.h"
#include "persistence.h"

class Commands {
public:
    Commands(Context &_ctx);

    void listen();

    void storeProps();

private:
    Context *ctx;
    Persistence *persist;

    void printProperty(uint8_t i);
};

#endif
