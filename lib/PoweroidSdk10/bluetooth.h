#ifndef BT_H
#define BT_H

#include <Arduino.h>

#define HISPEED 115200
#define VER "hc01.comV2."

typedef struct Bt {

  char *name;
  char *result;
  bool on = false;

  Bt(char * n);

  void getResult();

  void setPin(char * pin);

};

#endif
