#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <Arduino.h>

typedef struct Property {

    const __FlashStringHelper *desc;
    long val;
    long minv;
    long maxv;
    long scale = 1;

    Property(const __FlashStringHelper *a, long v, long n, long m, long s) : desc(a), minv(n), maxv(m), val(v),
                                                                            scale(s) {}
};

#endif
