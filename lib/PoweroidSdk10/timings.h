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
    TimingState(unsigned long interval) : interval(interval) {};
    void reset();

    unsigned long getCurrent();

    bool testInterval(unsigned long current);

   bool countdown(bool on, bool suspend, bool cancel);

    bool isTimeAfter(bool trigger);

    void flash(uint8_t pin, boolean on);

    bool ping();
};

#endif
