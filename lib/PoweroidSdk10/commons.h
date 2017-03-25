#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#define DHTPIN 5
#define PWR21
#define LED_PIN 13
#define FACTORY_RESET_PIN 9

#include <Arduino.h>
#include "properties.h"
#include "sensors.h"

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

typedef struct Context {
    const char *SIGNATURE;
    const char *version;
    Sensors *SENS;
    Property *FACTORY;
    long *RUNTIME;
    const uint8_t props_size;
    const char *id;
} Context;

unsigned long hash(byte *data, unsigned long size);

#endif

