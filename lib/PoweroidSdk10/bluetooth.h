#ifndef BT_H
#define BT_H

#include <Arduino.h>
#include <context.h>

#define BT_VER_05 "+VERSION:hc01.comV2.1"
#define BT_VER_06 "hc01.comV2.0"
#define PASSWD "1234"


#define HISPEED 115200

class Bt {
public:
  const char *name;
  char result[32];
  bool on = false;
  Bt(const char *id);

  void getResult();
  bool getConnected();
  bool getPassive();
  void setPin(char * pin);
private:
    bool connected = false;
    bool passive = false;

    String execBtAtCommand(const __FlashStringHelper *cmd);
    String execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2);
    String execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout);

    bool isConnected();
};

#endif
