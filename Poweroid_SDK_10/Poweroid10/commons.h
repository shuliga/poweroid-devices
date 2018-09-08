#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#define PWR23

//#define DEBUG
//#define WATCH_DOG

#ifdef DEBUG
    #define SSERIAL
    #define NO_CONTROLLER
#endif

#define LED_PIN 13
#define FACTORY_RESET_PIN 10

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "properties.h"

#ifdef PWR23

#define BOARD_VERSION "PWR23"

#define DHT_PIN 5

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

#endif // PWR23

#ifdef PWR20

#define BOARD_VERSION "PWR20"

#define ENC_BTN_PIN 7

#define DHT_PIN 2

#define IN1_PIN 2
#define IN2_PIN 4
#define IN3_PIN 6

#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#define PWR1_PIN 10
#define PWR2_PIN 11

#endif // PWR20

#define DEBOUNCE_DELAY 500L
#define SERIAL_READ_TIMEOUT 150
#define CONNECTION_CHECK 10000L


#define RX_SS 8
#define TX_SS 9

#define MODE_SERVER "srv"
#define MODE_CLIENT "cnt"
#define MODE_ASK "ask"

#ifdef SSERIAL
extern SoftwareSerial SSerial;
#endif

unsigned long hash(byte *data, unsigned long size);
char * idxToChar(uint8_t idx);
void writeLog(const char level, const char *origin, const int code);
void writeLog(const char level, const char *origin, const int code, unsigned long result);
void writeLog(const char level, const char *origin, const int code, const char *result);
uint8_t flashStringHelperToChar(const __FlashStringHelper *ifsh, char *dst);
uint8_t getNumberOfDigits(long i);
void noInfoToBuff();
#endif
