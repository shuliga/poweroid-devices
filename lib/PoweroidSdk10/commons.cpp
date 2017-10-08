#include <Arduino.h>
#include "commons.h"

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
  Serial.println(buff);
}

void writeLog(const char level, const char *origin, const int code, long result){
  char buff[32];
  sprintf(buff, "%c [%s] %i (%li)", level, origin, code, result);
  Serial.println(buff);
}

void writeLog(const char level, const char *origin, const int code, char *result){
  char buff[32];
  sprintf(buff, "%c [%s] %i '%s'", level, origin, code, result);
  Serial.println(buff);
}
