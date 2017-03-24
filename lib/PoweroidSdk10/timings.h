#ifndef TIMINGS_H
#define TIMINGS_H
#include <Arduino.h>

const unsigned long MAX_LONG = 4294967295L;

typedef struct TimingState{
    unsigned long mils;
    unsigned long interval;
    long delta;
    long suspended;
    bool state;
    bool dirty;
    TimingState(unsigned long _int) : interval(_int) {};
} TimingState;

unsigned long getCurrent(TimingState *ts);

bool testInterval(TimingState *ts, unsigned long current);

bool countdown(TimingState *ts, bool on, bool cancel);

bool countdown(TimingState *ts, bool on, bool suspend, bool cancel);

bool isTimeAfter(TimingState *ts, bool trigger);

void flash(TimingState *ts, uint8_t pin, bool on);

bool ping(TimingState *ts);

#endif
