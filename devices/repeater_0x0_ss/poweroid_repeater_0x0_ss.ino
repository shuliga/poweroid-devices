
#define ID "PWR-REP-SS"

#include <Poweroid10.h>
#include "poweroid_repeater_0x0_ss_state.h"
#include "poweroid_repeater_0x0_ss_prop.h"

#ifndef SSERIAL
#error This device cannot be run without defined SSERIAL
#endif

#ifndef NO_CONTROLLER
#error This device cannot be run with CONTROLLER enabled
#endif

#ifndef S_BUFF_LEN
#define S_BUFF_LEN 64
#endif


#ifdef MINI
#error This device cannot be run on MINI board versions
#endif

Context CTX = Context(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

char S_BUFF[S_BUFF_LEN];

void applyTimings() {
}

void processSensors() {

    while (Serial.available()) {
        uint8_t len = Serial.readBytes(S_BUFF, min(SSerial.availableForWrite(), min(Serial.available(), S_BUFF_LEN)));
        SSerial.write(S_BUFF, len);
        SSerial.flush();
    }

    while (SSerial.available()) {
        uint8_t len = SSerial.readBytes(S_BUFF, min(SSerial.available(), S_BUFF_LEN));
        Serial.write(S_BUFF, len);
        Serial.flush();
    }
}

void fillOutput() {
        BANNER.mode = 0;
        sprintf(BANNER.data.text, "%u:%u", S_BUFF_LEN, PROPS.FACTORY[0].runtime);
}

void runPowerStates() {
}

void setup() {
    PWR.begin();
    PWR_FLAGS = 8; // Enable TOKEN mode, thus disable serial output

    Serial.end();
    Serial.begin(PROPS.FACTORY[0].val);

    SSerial.end();
    SSerial.begin(PROPS.FACTORY[1].val);
}

void loop() {
    PWR.run();
}
