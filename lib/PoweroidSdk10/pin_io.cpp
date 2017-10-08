#include "Arduino.h"

void led(uint8_t no, bool on){
  if(on) {
    digitalWrite(no, HIGH);
  } else {
    digitalWrite(no, LOW);
  }
}

void pin_inv(uint8_t no, bool on){
  if(on) {
    digitalWrite(no, LOW);
  } else {
    digitalWrite(no, HIGH);
  }
}


