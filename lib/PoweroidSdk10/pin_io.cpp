#include "Arduino.h"

bool readPinLow(int pin){
  return digitalRead(pin) == LOW;
}

bool readPinHigh(int pin){
  return digitalRead(pin) == HIGH;
}

int readPinVal(int pin){
  return analogRead(pin);
}

void led(int no, bool on){
  if(on) {
    digitalWrite(no, HIGH);
  } else {
    digitalWrite(no, LOW);
  }
}

void pin_inv(int no, bool on){
  if(on) {
    digitalWrite(no, LOW);
  } else {
    digitalWrite(no, HIGH);
  }
}


