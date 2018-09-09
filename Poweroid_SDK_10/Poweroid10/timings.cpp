#include "timings.h"

unsigned long TimingState::getCurrent() {
    unsigned long current = millis();
    if (current < mils) {
        delta = -(MAX_LONG - mils);
        mils = 0;
    } else {
        delta = 0;
    }
    return current;
}

bool TimingState::testInterval(unsigned long current) {
    return (current - mils - delta) >= interval + (suspended == 0 ? 0 : current - suspended);
}

bool TimingState::countdown(bool on, bool suspend, bool cancel) {
    unsigned long current = getCurrent();
    if (!state && !dirty && on) {
        mils = current;
        state = true;
    }
    if (dirty && !on) {
        dirty = false;
    }
    if (state){
        if(suspend){
            suspended = getCurrent();
        } else {
            suspended = 0;
        }
        if ((testInterval(current) || cancel)) {
            mils = 0;
            state = false;
            dirty = on;
        }
    }
    return state;
}

bool TimingState::isTimeAfter(bool trigger) {
    unsigned long current = getCurrent();
    if (trigger) {
        if (testInterval(current)) {
            state = true;
        }
    } else {
        mils = current;
        state = false;
    }
    return state;
}

bool TimingState::flash() {
    unsigned long current = getCurrent();
    if (testInterval(current)) {
        state = !state;
        mils = current;
    }
    return state;
}

bool TimingState::ping() {
    unsigned long current = getCurrent();
    if (testInterval(current)) {
        mils = current;
        return true;
    } else {
        return false;
    }
}

void TimingState::reset() {
    mils = getCurrent();
    suspended = 0;
    state = false;
    dirty = false;
}
