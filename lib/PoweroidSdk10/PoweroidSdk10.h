#ifndef POWEROID_SDK_10_H
#define POWEROID_SDK_10_H

#define VERSION "POWEROID SDK 1.0"


#ifndef DHT_PIN
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

#define FULL_VERSION VERSION "-" BOARD_VERSION

const char SIGNATURE[] = "PWR";

static const int DEFAULT_BAUD = 9600;

class Pwr {
public:
    Context *CTX;
    Relays *REL;
    Sensors *SENS;
    Commands *CMD;
    Controller *CTRL;
    Bt *BT;

    Pwr(Context &ctx, Commands *_cmd, Controller *_ctrl, Bt *_bt);

    void begin();

    void run();

    void printVersion();

    void power(uint8_t i, bool i1);

private:

    bool firstRun = true;

    void init_pins();

    void loadDisarmedStates();
};

#endif //POWEROID_SDK_10_H
