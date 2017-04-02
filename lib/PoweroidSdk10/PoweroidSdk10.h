#ifndef POWEROID_SDK_10_H
#define POWEROID_SDK_10_H

#define VERSION "POWEROID SDK 1.0"

#ifndef DHTPIN
    #define DHTPIN 5
#endif
#ifndef ENC_BTN_PIN
    #define ENC_BTN_PIN 7
#endif


#include "commons.h"
#include "context.h"
#include "properties.h"
#include "relays.h"
#include "sensors.h"
#include "bluetooth.h"
#include "context.h"
#include "commands.h"
#include "controller.h"

static char SIGNATURE[] = "PWR";
static char version[] = VERSION;

class Pwr {
public:
    Context *CTX;
    Sensors *SENS;
    Bt *BT;
    Relays REL;
    Commands CMD;
    Controller CTRL;

    Pwr(Context &ctx);

    void begin();

    void processSensors();

    void run();

    void printVersion();

private:
    void init_outputs();

    void init_inputs();

};

#endif
