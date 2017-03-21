#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#define PWR21


#include <Arduino.h>

#ifdef PWR21

#define ENC1_PIN 2
#define ENC2_PIN 3
#define ENC_BTN_PIN 4

#define IN1_PIN 5
#define IN2_PIN 6
#define IN3_PIN 7

#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#else

#define ENC_BTN_PIN 7

#define IN1_PIN 2
#define IN2_PIN 4
#define IN3_PIN 6

#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#endif // PWR20/PWR21

const uint8_t IN_PINS[] = {IN1_PIN, IN2_PIN, IN3_PIN};
const uint8_t INA_PINS[] = {INA1_PIN, INA2_PIN, INA3_PIN};


String printState(String states[], int idx);
String printSensor(boolean states[], int idx);
unsigned long hash(byte* data, unsigned long size);

#endif

