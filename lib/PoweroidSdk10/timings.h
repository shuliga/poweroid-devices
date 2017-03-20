#ifndef TIMINGS_H
#define TIMINGS_H

const unsigned long MAX_LONG = 4294967295L;

struct TimingState{
  unsigned long mils;
  unsigned long interval;
  long delta;
  long fact;
  bool state;
  bool dirty;
};

unsigned long getCurrent(TimingState* ts);
bool testInterval(TimingState* ts, unsigned long current);
bool countdown(TimingState* ts, bool on, bool cancel);
bool isTimeAfter(TimingState* ts, bool trigger);
void flash(TimingState* ts, int pin, bool on);

#endif
