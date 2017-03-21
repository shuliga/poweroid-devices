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

    Property() {}

    Property(Property *_p) {
        desc = _p->desc;
        val = _p->val;
        minv = _p->minv;
        maxv = _p->maxv;
        scale = _p->scale;
    }

    Property(const __FlashStringHelper *a, long n, long m) : desc(a), minv(n), maxv(m) {}

    Property(const __FlashStringHelper *a, long n, long m, long v) : desc(a), minv(n), maxv(m), val(v) {}

    Property(const __FlashStringHelper *a, long v, long n, long m, long s) : desc(a), minv(n), maxv(m), val(v),
                                                                            scale(s) {}

    long inc() {
        return inc(1);
    }

    long dec() {
        return dec(1);
    }

    long inc_accel() {
        return inc(PROPERTIES_STEP_ACCEL);
    }

    long dec_accel() {
        return dec(PROPERTIES_STEP_ACCEL);
    }


private:
    long inc(int stp) {
        long new_val = val + (scale * stp);
        if (new_val <= maxv) {
            val = new_val;
        }
        return val;
    }

    long dec(int stp) {
        long new_val = val - (scale * stp);
        if (new_val >= maxv) {
            val = new_val;
        }
        return val;
    }

} ;

typedef struct {
        Property *FACTORY;
        long *RUNTIME;
        int props_size;
        String *states;
        int states_size;
        char *id;
} Context;


#endif
