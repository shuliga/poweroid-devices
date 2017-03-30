#include "timings.h"

#include <Arduino.h>

unsigned long getCurrent(TimingState &ts) {
    unsigned long current = millis();
    if (current < ts.mils) {
        ts.delta = -(MAX_LONG - ts.mils);
        ts.mils = 0;
    } else {
        ts.delta = 0;
    }
    return current;
}

bool testInterval(TimingState &ts, unsigned long current) {
    return (current - ts.mils - ts.delta) >= ts.interval + (ts.suspended == 0 ? 0 : current - ts.suspended);
}

bool countdown(TimingState &ts, bool on, bool cancel) {
    unsigned long current = getCurrent(ts);
    if (!ts.state && !ts.dirty && on) {
        ts.mils = current;
        ts.state = true;
    }
    if (ts.dirty && !on) {
        ts.dirty = false;
    }
    if (ts.state && (testInterval(ts, current) || cancel)) {
        ts.mils = 0;
        ts.state = false;
        ts.dirty = on;
    }
    return ts.state;
}

bool countdown(TimingState &ts, bool on, bool suspend, bool cancel) {
    unsigned long current = getCurrent(ts);
    if (!ts.state && !ts.dirty && on) {
        ts.mils = current;
        ts.state = true;
    }
    if (ts.dirty && !on) {
        ts.dirty = false;
    }
    if (ts.state){
        if(suspend){
            ts.suspended = getCurrent(ts);
        } else {
            ts.suspended = 0;
        }
        if ((testInterval(ts, current) || cancel)) {
            ts.mils = 0;
            ts.state = false;
            ts.dirty = on;
        }
    }
    return ts.state;
}

bool isTimeAfter(TimingState &ts, bool trigger) {
    unsigned long current = getCurrent(ts);
    if (trigger) {
        if (testInterval(ts, current)) {
            ts.state = true;
        }
    } else {
        ts.mils = current;
        ts.state = false;
    }
    return ts.state;
}

void flash(TimingState &ts, uint8_t pin, boolean on) {
    unsigned long current = getCurrent(ts);
    if (testInterval(ts, current)) {
        ts.state = !ts.state;
        ts.mils = current;
    }
    int out;
    if (ts.state && on) {
        out = HIGH;
    } else {
        out = LOW;
    }
    digitalWrite(pin, out);
}

bool ping(TimingState &ts) {
    unsigned long current = getCurrent(ts);
    if (testInterval(ts, current)) {
        ts.mils = current;
        return true;
    } else {
        return false;
    }
}

void TimingState::reset() {
    mils = 0;
    delta = 0;
    suspended = 0;
    state = false;
    dirty = false;
}
