//
// Created by SHL on 26.03.2017.
//

#ifndef POWEROID_10_CONTEXT_H
#define POWEROID_10_CONTEXT_H

#include "sensors.h"
#include "relays.h"
#include "persistence.h"

typedef struct Context {
    Context(const char *_signature, const char *_version, Property *_factory, long *_runtime, const uint8_t _props_size,
            const char *_id, const uint8_t _states_size, char *(*_printState)(uint8_t i),
            void (*_disarmState)(uint8_t i, bool _disarm), int8_t _defaultPropIdx)
            : signature(_signature), version(_version), FACTORY(_factory), RUNTIME(_runtime), SENS(), RELAYS(),
              props_size(_props_size), id(_id), states_size(_states_size), PERS(Persistence(_signature, _runtime, _props_size, RELAYS.mappings, VIRTUAL_RELAYS)),
              printState(_printState), disarmState(_disarmState), defaultPropertyIdx(_defaultPropIdx) {}

    const char *signature;
    const char *version;

    Sensors SENS;
    Relays RELAYS;

    Property *FACTORY;
    long *RUNTIME;
    const uint8_t props_size;
    const char *id;
    const uint8_t states_size;

    Persistence PERS;

    char *(*printState)(uint8_t i);

    void (*disarmState)(uint8_t i, bool _disarm);

    int8_t defaultPropertyIdx;
    bool refreshProps;
    bool passive;
};

#endif //POWEROID_10_CONTEXT_H
