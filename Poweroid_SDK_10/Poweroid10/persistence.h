#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "properties.h"

#define STORAGE_MAX_SIZE 64
#define BASE 0

#define SIGNATURE_SIZE 4
#define HASH_SIZE sizeof(unsigned long)
#define STATES_SIZE 1
#define MAPPINGS_SIZE 8

#define HASH_OFFSET (BASE + SIGNATURE_SIZE)
#define STATES_OFFSET (HASH_OFFSET + HASH_SIZE)
#define MAPPINGS_OFFSET (STATES_OFFSET + STATES_SIZE)
#define PROPS_OFFSET (MAPPINGS_OFFSET + MAPPINGS_SIZE)
#define PROP_ADDR(x) PROPS_OFFSET + sizeof(long)*(x)

#define DEVICE_MODE_STATE 7


struct Persistence {

    const char *const ORIGIN = "EEPROM";
    uint8_t size;
    uint8_t mappings_size;
        char signature[SIGNATURE_SIZE];
    const char *given_sign_chr;
    Property *props;
    int8_t *mappings;

    Persistence(const char *_sign, Property *props, uint8_t props_size, int8_t *mappings, uint8_t msz);

    void storeProperties(Property *props);

    void storeState(uint8_t id, bool state);

    void storeValue(uint8_t i, long val);

    bool loadState(uint8_t id);

    void loadProperties(Property *prop);

    void checkFactoryReset(Property *props);

    void storeMappings(int8_t *mappings);

    void loadMappings(int8_t *mappings);

    void begin();
};


#endif
