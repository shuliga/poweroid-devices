#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <Arduino.h>
#include <avr/pgmspace.h>

#define PROPERTIES_STEP_ACCEL 5

typedef struct Property {

    const __FlashStringHelper *desc;
    long val;
    long minv;
    long maxv;
    long scale = 1;

    Property(Property *_p) {
        desc = _p->desc;
        val = _p->val;
        minv = _p->minv;
        maxv = _p->maxv;
        scale = _p->scale;
    }

    Property(const __FlashStringHelper *a, long v, long n, long m, long s) : desc(a), minv(n), maxv(m), val(v),
                                                                            scale(s) {}

} ;

#endif
