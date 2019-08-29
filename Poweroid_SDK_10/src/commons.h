#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "global.h"

#define PWR23

//#define SPI
//#define SAVE_RAM

//#define DEBUG
//#define WATCH_DOG

#ifdef PWR23

#define BOARD_VERSION "PWR23"

//Encoder pins
#define ENC2_PIN 2
#define ENC1_PIN 3
#define ENC_BTN_PIN 4

//Discrete signal IN pins
#define DHT_PIN 5 //DHT sensor default pin
#define IN1_PIN 5
#define IN2_PIN 6
#define IN3_PIN 7

//Relay pins
#define PWR1_PIN 8
#define PWR2_PIN 9

#undef FACTORY_RESET_PIN 13

#ifndef SPI
#define IND1_PIN 10
#define IND2_PIN 11
#define IND3_PIN 12
#define LED_PIN 13
#endif

//Analogue signal IN pins
#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16
#define INA4_PIN 17

#ifdef SPI
#define SPI_SS_PIN 10
#endif

#endif // PWR23

#ifdef PWR20

#define BOARD_VERSION "PWR20"

//Discrete signal IN pins
#define DHT_PIN 2

#define IN1_PIN 2
#define IN2_PIN 4
#define IN3_PIN 6

#define ENC_BTN_PIN 7

//Relay pins
#define PWR1_PIN 10
#define PWR2_PIN 11

//Analogue signal IN pins
#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#endif // PWR20


#define FLAG_RELAY_ON_LOW       1
#define FLAG_REMOTE_ENABLE      2
#define FLAG_REMOTE_SERVER      3
#define FLAG_TOKEN_ENABLE       4
#define FLAG_LOW_SPEED          5
#define FLAG_REF_1V1            6

#define FLAGS_MAX 63
#define TOKEN_MAX 8

#define ExtractFlag(flag)   PWR_FLAGS >> ((flag) - 1) & 0x1

#define RELAY_ON_LOW        ExtractFlag(FLAG_RELAY_ON_LOW)
#define REMOTE_ENABLE       ExtractFlag(FLAG_REMOTE_ENABLE)
#define REMOTE_SERVER       ExtractFlag(FLAG_REMOTE_SERVER)
#define TOKEN_ENABLE        ExtractFlag(FLAG_TOKEN_ENABLE)
#define LOW_SPEED           ExtractFlag(FLAG_LOW_SPEED)
#define REF_1V1             ExtractFlag(FLAG_REF_1V1)

#define DEBOUNCE_DELAY 500L
#define SERIAL_READ_TIMEOUT 150
#define CONNECTION_CHECK 5000L


#define RX_SS 8
#define TX_SS 9

#define REMOTE_CONTROL "ctrl"
#define REMOTE_HOST "host"

#define CHAR_CONNECTED      130
#define CHAR_DISCONNECTED   129

#ifdef SSERIAL
extern SoftwareSerial SSerial;
#endif

#ifdef DEBUG
#define SSERIAL
#define NO_CONTROLLER
#endif

#ifdef NO_CONTROLLER
#undef SAVE_RAM
#endif

unsigned long hash(byte *data, unsigned long size);

char *idxToChar(uint8_t idx);

void writeLog(const char level, const char *origin, const int code, unsigned long result);

void writeLog(const char level, const char *origin, const int code, const char *result);

void writeLog(const char level, const char *origin, const int code);

uint8_t flashStringHelperToChar(const __FlashStringHelper *ifsh, char *dst);

void noInfoToBuff();

#endif // COMMONS_H