#include <Arduino.h>
#include <src/SoftwareSerial.h>
#include "commons.h"

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

void writeLog(const char level, const char *origin, const int code){
  char buff[32];
  sprintf(buff, "%c [%s] %i", level, origin, code);
#ifdef SSERIAL
    SSerial.println(buff);
#else
    Serial.println(buff);
#endif
}

void writeLog(const char level, const char *origin, const int code, long result){
  char buff[32];
  sprintf(buff, "%c [%s] %i (%li)", level, origin, code, result);
#ifdef SSERIAL
    SSerial.println(buff);
#else
    Serial.println(buff);
#endif
}

void writeLog(const char level, const char *origin, const int code, char *result){
  char buff[32];
  sprintf(buff, "%c [%s] %i '%s'", level, origin, code, result);
#ifdef SSERIAL
    SSerial.println(buff);
#else
    Serial.println(buff);
#endif
}
