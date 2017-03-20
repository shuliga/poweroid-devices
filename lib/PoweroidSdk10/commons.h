#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#include <Arduino.h>

String printState(String states[], int idx);
String printSensor(boolean states[], int idx);
unsigned long hash(byte* data, unsigned long size);

#endif

