#ifndef BT_H
#define BT_H

#include <Arduino.h>
#include <context.h>

#ifdef SSERIAL
#include <SoftwareSerial.h>
#endif

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

#define HC_05_AT_BAUD 38400
#define HC_05_AT_BAUD_FAST 57600
#define HC_05_AT_UART "AT+UART="
#define HC_05_AT_BAUD_AT HC_05_AT_UART STRINGIZE(HC_05_AT_BAUD_FAST) ",0,0"
#define HC_06_BAUD 115200


#define BT_VER_05 "+VERSION:hc01.comV2.1"
#define BT_VER_06 "hc01.comV2.0"
#define PASSWD "1234"


#define HISPEED 115200

const int TIMEOUT_STEP = 100;

class Bt {
public:
  const char *name;
  bool server = false;
  Bt(const char *id);

  void begin();

private:

    String execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout);
    String execBtAtCommand(const __FlashStringHelper *cmd);

    void applyBt05();

    void execReset();

    bool checkPeerType(const char *conn_type);

    String getVerHC06() const;

    void cleanSerial() const;
};

#endif // BT_H
