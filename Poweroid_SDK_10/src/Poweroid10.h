#ifndef POWEROID_SDK_10_H
#define POWEROID_SDK_10_H

#define VERSION "POWEROID SDK 1.0"


#ifndef DHT_PIN
    #define DHT_PIN 5
#endif
#ifndef ENC_BTN_PIN
    #define ENC_BTN_PIN 7
#endif

#if defined(__AVR_ATmega1284P__)
#define HZ_TIMER_CONST 19530  // 65536 - 19530; // 20000000L / 1024 / FRQ - 1; // set timer value 20MHz/1024/1Hz-1
#else
#define HZ_TIMER_CONST 15624  // 65536 - 15624; // 16000000L / 1024 / FRQ - 1; // set timer value 16MHz/1024/1Hz-1
#endif

#include "commons.h"
#include "context.h"
#include "properties.h"
#include "relays.h"
#include "sensors.h"
#include "bluetooth.h"
#include "context.h"
#include "commander.h"
#include "controller.h"
#include "indicators.h"


#define FULL_VERSION VERSION "-" BOARD_VERSION

const char SIGNATURE[] = "PWR";


extern void applyTimings();
extern void fillOutput();
extern void processSensors();
extern void runPowerStates();

const int DEFAULT_BAUD = 9600;

class Pwr {
public:
    Context *CTX;
    Relays *REL;
    Sensors *SENS;
    Commander *CMD;
    Controller *CTRL;

    Bt *BT;

    Pwr(Context &ctx, Commander *_cmd, Controller *_ctrl, Bt *_bt);

    void begin();

    void run();

    void printVersion();

    void power(uint8_t i, bool i1);

private:

    bool firstRun = true;

    void initPins();

    void printChangedStates();

    void loadDisarmedStates();

};

#ifdef DEBUG
void initTimer();
#endif

#endif //POWEROID_SDK_10_H
