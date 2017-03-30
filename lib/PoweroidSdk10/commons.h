#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#define PWR21

#define LED_PIN 13
#define FACTORY_RESET_PIN 9

#include <Arduino.h>
#include "properties.h"

#ifdef PWR21

#define BOARD_VERSION "PWR21"

#define DHTPIN 5

#define ENC1_PIN 2
#define ENC2_PIN 3
#define ENC_BTN_PIN 4

#define IN1_PIN 5
#define IN2_PIN 6
#define IN3_PIN 7

#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#define PWR1_PIN 8
#define PWR2_PIN 9

#endif // PWR21

#ifdef PWR20

#define BOARD_VERSION "PWR20"

#define ENC_BTN_PIN 7

#define DHTPIN 2

#define IN1_PIN 2
#define IN2_PIN 4
#define IN3_PIN 6

#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#define PWR1_PIN 10
#define PWR2_PIN 11

#endif // PWR20

//extern const uint8_t IN_PINS[];
//extern const uint8_t INA_PINS[];
//extern const uint8_t OUT_PINS[];

unsigned long hash(byte *data, unsigned long size);

#endif

