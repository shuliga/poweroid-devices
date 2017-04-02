#ifndef BT_H
#define BT_H

#include <Arduino.h>

#define HISPEED 115200
#define BT_VER "hc01.comV2."

typedef struct Bt {

  const char *name;
  char result[32];
  bool on = false;

  Bt(const char * n);

  void getResult();

  void setPin(char * pin);

};

#endif
