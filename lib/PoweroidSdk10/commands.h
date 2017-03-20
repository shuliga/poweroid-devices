#ifndef COMMANDS_H
#define COMMANDS_H

#define CMD_PREF_GET_STATE "get_state_"
#define CMD_GET_STATE_ALL "get_state_all"
#define CMD_PREF_GET_PROP "get_prop_"
#define CMD_PREF_SET_PROP "set_prop_"
#define CMD_GET_PROP_ALL "get_prop_all"
#define CMD_GET_PROP_LEN "get_prop_len"
#define CMD_PREF_LOAD_PROPS "load_props"
#define CMD_PREF_STORE_PROPS "store_props"
#define CMD_RESET_PROPS "reset_props"
#define CMD_GET_SENSOR_ALL "get_sensor_all"

// #define BT // ENABLE BLUETOOTH

#include "properties.h"
#include "persistence.h"
#include "bluetooth.h"


struct Commands{
  
  char *id;
  Property *props_factory;
  long *props_runtime;
  int props_size;
  String *states;
  int states_size;
  Persistence persist;
  Bt bt;

  Commands(){}
  Commands(Property *p, long *p_r, int ps, String *s, int ss, char * _id);

  void printProperty(int i);
  void listen();
private:
  void processRotary();
  void initRotaryInterrupts();
  void initDisplay();
  void storeProps(String cmd);
    void outputStatus(const __FlashStringHelper *txt, const int long);
    void printPropVal(int _prop_val, bool brackets);
    void printPropDescr(uint8_t _idx);
};

#endif
