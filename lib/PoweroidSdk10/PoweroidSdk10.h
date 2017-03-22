#ifndef POWEROID_SDK_10_H
#define POWEROID_SDK_10_H

#define VERSION "POWEROID SDK 1.0"
#define PWR21

#define BT

#include "commons.h"
#include "properties.h"
#include "sensors.h"
#include "commands.h"
#include "controller.h"
#include "bluetooth.h"

static char SIGNATURE[] = "PWR";
static char version[] = VERSION;

const uint8_t PWR1_PIN = 10;
const uint8_t PWR2_PIN = 11;

const uint8_t OUT_PINS[] = {PWR1_PIN, PWR2_PIN};

class Pwr {
public:
    Context *CTX;
    Bt *bt;
    Commands CMD;
    Controller CTRL;

    Pwr(Context *ctx);

    void begin();

    void run();

    void printVersion();

private:
    void init_outputs();

    void init_inputs();

};

#endif
