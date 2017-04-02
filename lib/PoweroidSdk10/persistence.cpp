
#include <EEPROM.h>
#include "pin_io.h"
#include "commons.h"
#include "persistence.h"

unsigned long hashProp(long *props, int size) {
      return hash((byte *)props, size * sizeof(long));
}

Persistence::Persistence(const String s, long *props_runtime, int sz){
  checkFactoryReset(props_runtime);
  size = sz;
  EEPROM.get(BASE, signature);
  String sign = String(signature);
  unsigned long eeprom_hash;
  EEPROM.get(BASE + SIGNATURE_SIZE, eeprom_hash);
  Serial.print(F("EEPROM signature '"));
  Serial.print(sign + "', hash:");
  Serial.println(eeprom_hash);
  if (sign != s){
    s.toCharArray(signature, SIGNATURE_SIZE);
    EEPROM.put(BASE, signature);
    Serial.print(F("Not a native EEPROM. Overwritting signature '"));
    Serial.println(sign + "' with: '" + s + "'");
    storeProperties(props_runtime);
    return;
  }
  unsigned long hash = hashProp(props_runtime, size);
  if (eeprom_hash != hash){
    Serial.println(F("EEPROM hash differs from factory"));
    loadProperties(props_runtime);
  }
  
}


void Persistence::storeProperties(long *props){
  for(uint8_t i=0; i < size; i++){
    storeValue(i, props[i]);
  }
  unsigned long hash = hashProp(props, size);
  char c_hash[11];
  c_hash[10] = 0;
  ultoa(hash, c_hash, 10);
  Serial.print(size);
  Serial.print(F(" properties stored in EEPROM. Hash: "));
  Serial.println(c_hash);
  EEPROM.put(BASE + SIGNATURE_SIZE, hash);
}

void Persistence::storeValue(long *prop){
}

void Persistence::storeValue(uint8_t i, long val){
  if(i <= size){
    EEPROM.put(ADDR(i), val);
  }
}

void Persistence::loadValue(long *prop){
}

void Persistence::loadValue(uint8_t i){
}

void Persistence::loadProperties(long *props){
  for(uint8_t i = 0; i < size; i++){
     EEPROM.get(ADDR(i), props[i]);
  }
  Serial.print(size);
  Serial.println(F(" properties loaded form EEPROM"));
}

void Persistence::checkFactoryReset(long *props_runtime) {
  pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
  if(readPinLow(FACTORY_RESET_PIN)){
    storeProperties(props_runtime);
    Serial.println("Factory reset EEPROM");
  }
}
