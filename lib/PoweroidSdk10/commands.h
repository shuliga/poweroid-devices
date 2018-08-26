#ifndef COMMANDS_H
#define COMMANDS_H

#include "commons.h"
#include "persistence.h"
#include "context.h"
#include "common_commands.h"

class Commands {
public:

    Commands(Context &_ctx);

    void listen();

    void storeProps();

    void printChangedState(bool prev_state, bool state, uint8_t id);

    void disarmState(uint8_t i, bool disarm);

    bool isConnected();

private:

    bool connected = false;

    Context *ctx;

    const char * printProperty(uint8_t i);

    void printBinProperty(uint8_t i);

};

#endif // COMMANDS_H
