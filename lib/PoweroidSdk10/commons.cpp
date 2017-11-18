#include <Arduino.h>
#include <src/SoftwareSerial.h>
#include "commons.h"
#include "global.h"

#ifdef SSERIAL
SoftwareSerial SSerial = SoftwareSerial(RX_SS, TX_SS);
#endif

unsigned long hash(byte *data, unsigned long size) {
    unsigned long hash = 19;
    for (unsigned long i = 0; i < size; i++) {
        byte c = *data++;
        if (c != 0) { // usually when doing on strings this wouldn't be needed
            hash *= c;
        }
        hash += 7;
    }
    return hash;
}

void writeLog(const char level, const char *origin, const int code) {
    sprintf(BUFF, "%c [%s] %i", level, origin, code);
#ifdef SSERIAL
    SSerial.println(BUFF);
#else
    Serial.println(BUFF);
#endif
}

void writeLog(const char level, const char *origin, const int code, unsigned long result) {
    sprintf(BUFF, "%c [%s] %i (%lu)", level, origin, code, result);
#ifdef SSERIAL
    SSerial.println(BUFF);
#else
    Serial.println(BUFF);
#endif
}

void writeLog(const char level, const char *origin, const int code, const char *result) {
    sprintf(BUFF, "%c [%s] %i '%s'", level, origin, code, result);
#ifdef SSERIAL
    SSerial.println(BUFF);
#else
    Serial.println(BUFF);
#endif
}

uint8_t flashStringHelperToChar(const __FlashStringHelper *ifsh, char *dst) {
    PGM_P p = reinterpret_cast<PGM_P>(ifsh);
    uint8_t n = 0;
    while (1) {
        unsigned char c = pgm_read_byte(p++);
        dst[n++] = c;
        if (c == 0) break;
    }
    return n;
}