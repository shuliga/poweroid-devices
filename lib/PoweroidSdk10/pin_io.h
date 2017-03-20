#ifndef PIN_IO_H
#define PIN_IO_H

bool readPinLow(int pin);
bool readPinHigh(int pin);
int readPinVal(int pin);
void led(int no, bool on);
void pin_inv(int no, bool on);

#endif