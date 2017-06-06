#ifndef COMMANDS_H
#define COMMANDS_H

#include "commons.h"
#include "context.h"
#include "persistence.h"

struct CommandsStr {
    const char* CMD_GET_VER;
    const char* CMD_GET_DHT;
    const char* CMD_PREF_GET_STATE;
    const char* CMD_PREF_DISARM_STATE;
    const char* CMD_GET_STATE_ALL;
    const char* CMD_PREF_GET_PROP;
    const char* CMD_PREF_SET_PROP;
    const char* CMD_GET_PROP_ALL;
    const char* CMD_GET_PROP_LEN;
    const char* CMD_LOAD_PROPS;
    const char* CMD_STORE_PROPS;
    const char* CMD_RESET_PROPS;
    const char* CMD_GET_SENSOR_ALL;
    const char* CMD_GET_RELAY_ALL;
};

//struct CommandsStr {
//    const char CMD_GET_VER[8] = "get_ver";
//    const char CMD_GET_DHT[8] = "get_dht";
//    const char CMD_PREF_GET_STATE[11] =  "get_state_";
//    const char CMD_PREF_DISARM_STATE[14] = "disarm_state_";
//    const char CMD_GET_STATE_ALL[14] = "get_state_all";
//    const char CMD_PREF_GET_PROP[10] = "get_prop_";
//    const char CMD_PREF_SET_PROP[10] = "set_prop_";
//    const char CMD_GET_PROP_ALL[13] = "get_prop_all";
//    const char CMD_GET_PROP_LEN[13] = "get_prop_len";
//    const char CMD_LOAD_PROPS[11] = "load_props";
//    const char CMD_STORE_PROPS[12] =  "store_props";
//    const char CMD_RESET_PROPS[12] = "reset_props";
//    const char CMD_GET_SENSOR_ALL[15] = "get_sensor_all";
//    const char CMD_GET_RELAY_ALL[14] = "get_relay_all";
//
//};

class Commands {
public:

    CommandsStr cmd_str;

    Commands(Context &_ctx, Persistence &_pers);

    void listen();

    void storeProps();

private:
    Context *ctx;
    Persistence *persist;

    void printProperty(uint8_t i);
};

#endif // COMMANDS_H
