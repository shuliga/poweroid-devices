#include <Arduino.h>

String printState(String states[], int idx){
  return (String)"State[" + idx + (String)"]:" + states[idx];
}

String printSensor(boolean sensors[], int idx){
  return (String)"Sensor[" + idx + (String)"]:" + (String)(sensors[idx] ? "installed" : "not installed");
}

unsigned long hash(byte* data, unsigned long size) {
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


