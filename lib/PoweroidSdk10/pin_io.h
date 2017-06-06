#ifndef PIN_IO_H
#define PIN_IO_H

#include <Arduino.h>

#define LED_PIN
bool readPinLow(uint8_t pin);
bool readPinHigh(uint8_t pin);
int readPinVal(uint8_t pin);
void led(uint8_t no, bool on);
void pin_inv(uint8_t no, bool on);

#endif