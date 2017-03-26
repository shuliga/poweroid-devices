//
// Created by SHL on 26.03.2017.
//

#ifndef POWEROID_10_CONTEXT_H
#define POWEROID_10_CONTEXT_H

#include "sensors.h"

typedef struct Context {
    const char *SIGNATURE;
    const char *version;
    Sensors *SENS;
    Property *FACTORY;
    long *RUNTIME;
    const uint8_t props_size;
    const char *id;
    const uint8_t states_size;
    char* (*printState)(uint8_t i);
} Context;

#endif //POWEROID_10_CONTEXT_H
