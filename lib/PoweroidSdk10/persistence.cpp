#include "persistence.h"
#include <EEPROM.h>
#include "commons.h"

unsigned long hashProp(long *props, int size) {
      return hash((byte *)props, size);
}

Persistence::Persistence(String s, long *props_runtime, int sz){
  size = sz;
  EEPROM.get(0, signature);
  String sign = String(signature);
  Serial.print(F("EEPROM signature '"));
  Serial.println(sign + "'");
  if (sign != s){
    s.toCharArray(signature, SIGNATURE_SIZE);
    EEPROM.put(0, signature);
    Serial.print(F("Not a native EEPROM. Overwritting signature '"));
    Serial.println(sign + "' with: '" + s + "'");
    storeProperties(props_runtime);
    return;
  }
  unsigned long eeprom_hash;
  unsigned long hash = hashProp(props_runtime, size);
  EEPROM.get(SIGNATURE_SIZE, eeprom_hash);
  if (eeprom_hash != hash){
    Serial.println(F("EEPROM hash differs from factory"));
    loadProperties(props_runtime);
    Serial.println(F("Properties loaded from EEPROM"));
  }
  
}


void Persistence::storeProperties(long* props){
  for(int i=0; i < size; i++){
    storeValue(i, props[i]);
  }
  unsigned long hash = hashProp(props, size);
  char c_hash[11];
  c_hash[10] = 0;
  ultoa(hash, c_hash, 10);
  Serial.print(size);
  Serial.print(F(" properties stored in EEPROM. Hash:"));
  Serial.println(c_hash);
  EEPROM.put(SIGNATURE_SIZE, hash);
}

void Persistence::storeValue(long* prop){
}

void Persistence::storeValue(int i, long val){
  if(i <= size){
    EEPROM.put(ADDR(i), val);
  }
}

void Persistence::loadValue(long* prop){
}

void Persistence::loadValue(int i){
}

void Persistence::loadProperties(long* props){
  for(int i = 0; i < size; i++){
     EEPROM.get(ADDR(i), props[i]);
  }
}

