#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <Arduino.h>

typedef struct Property {

    const __FlashStringHelper *desc;
    const char *measure;
    long runtime;

    long val;
    long minv;
    long maxv;
    long scale = 1;

    Property(){}
    Property(long v, long n, long m, long s) : minv(n), maxv(m), val(v), scale(s) {}
};

#endif
