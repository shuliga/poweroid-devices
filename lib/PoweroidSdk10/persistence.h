#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "properties.h"

#define STORAGE_MAX_SIZE 64
#define BASE 20
#define SIGNATURE_SIZE 4
#define STATES_SIZE 1
#define MAPPINGS_SIZE 8
#define HASH_OFFSET (BASE + SIGNATURE_SIZE)
#define STATES_OFFSET (HASH_OFFSET + sizeof(unsigned long))
#define MAPPINGS_OFFSET (STATES_OFFSET + STATES_SIZE)
#define PROPS_OFFSET (MAPPINGS_OFFSET + MAPPINGS_SIZE)
#define ADDR(x) PROPS_OFFSET + sizeof(long)*(x)


struct Persistence {

    const char *const ORIGIN = "EEPROM";
    uint8_t size;
    uint8_t mappings_size;
    char signature[SIGNATURE_SIZE];
    String given_sign;
    long *props_runtime;
    uint8_t *mappings;

    Persistence(const String &_sign, long *props_runtime, uint8_t props_size, uint8_t *mappings, uint8_t msz);

    void storeProperties(long *props);

    void storeState(uint8_t id, bool state);

    void storeValue(uint8_t i, long val);

    bool loadState(uint8_t id);

    void loadProperties(long *prop);

    void checkFactoryReset(long *props_runtime);

    void storeMapping(uint8_t id, int8_t mapped_id);

    int8_t loadMapping(uint8_t id);

    void storeMappings(uint8_t *mappings);

    void loadMappings(uint8_t *mappings);

    void begin();
};


#endif
