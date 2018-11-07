//
// Created by SHL on 26.03.2017.
//

#ifndef POWEROID_10_CONTEXT_H
#define POWEROID_10_CONTEXT_H

#include "sensors.h"
#include "relays.h"
#include "persistence.h"

typedef struct RunState {
    RunState() = default;

    uint8_t idx;
    char *name;
    char *state;

};

extern uint8_t state_count;

RunState *getState(uint8_t i);
void disarmState(uint8_t i, bool _disarm);
const char * printState(uint8_t i);

typedef struct Context {
    Context(const char *_signature, const char *_version, Property *_factory_props, const uint8_t _props_size,
            const char *_id, int8_t _defaultPropIdx, const char * (*_printBanner)())
            : signature(_signature), version(_version), PROPERTIES(_factory_props),
              props_size(_props_size), id(_id), SENS(), RELAYS(),
              PERS(Persistence(_signature, _factory_props, _props_size)),
              props_default_idx(_defaultPropIdx), printBanner(_printBanner) {}

    const char *signature;
    const char *version;

    Sensors SENS;
    Relays RELAYS;

    Property *PROPERTIES;
    Property remoteProperty;
    uint8_t props_size;
    const char *id;

    Persistence PERS;

    const char * (*printBanner)();

    int8_t props_default_idx;
    bool refreshProps;
    bool refreshState;
    bool passive;
    bool connected = false;
    bool bt = false;
    bool peerFound;

    boolean canAccessLocally(){
        return !bt || !passive;
    }

};

#endif //POWEROID_10_CONTEXT_H
