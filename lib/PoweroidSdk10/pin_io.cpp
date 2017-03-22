#include "Arduino.h"

bool readPinLow(uint8_t pin){
  return digitalRead(pin) == LOW;
}

bool readPinHigh(uint8_t pin){
  return digitalRead(pin) == HIGH;
}

int readPinVal(uint8_t pin){
  return analogRead(pin);
}

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


