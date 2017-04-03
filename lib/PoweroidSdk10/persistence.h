#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "properties.h"

#define STORAGE_MAX_SIZE 64
#define BASE 100
#define SIGNATURE_SIZE 4
#define STATES_SIZE 1
#define STATES_OFFSET (BASE + SIGNATURE_SIZE + sizeof(unsigned long))
#define HEADER_OFFSET (STATES_OFFSET + STATES_SIZE)
#define ADDR(x) HEADER_OFFSET + sizeof(long)*(x)

struct Persistence {

    int size;
    char signature[SIGNATURE_SIZE];

    Persistence(String s, long *props_runtime, int props_size);

    void storeProperties(long *props);

    void storeState(uint8_t id, bool state);

    void storeValue(uint8_t i, long val);

    bool loadState(uint8_t id);

    void loadProperties(long *prop);

    void checkFactoryReset(long *props_runtime);
};


#endif
