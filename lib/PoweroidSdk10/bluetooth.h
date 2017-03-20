#ifndef BT_H
#define BT_H

#include <Arduino.h>

#define HISPEED 115200
#define VER "hc01.comV2."

struct Bt {

  char *name;
  String result;
  bool on = false;

  Bt();
  
  Bt(char * n);

  void getResult();

  void setPin(char * pin);

  bool isOn();

};

#endif
