
#include <EEPROM.h>
#include "pin_io.h"
#include "commons.h"
#include "persistence.h"

// Console output codes
// 100 - Signature
// 101 - Hash
// 200 - Factory reset
// 201 - Properties stored in EEPROM.
// 202 - Properties loaded from EEPROM
// 301 - Hash differs from factory.
// 501 - Not a native EEPROM. Overwriting signature.

unsigned long hashProp(Property *props, int size) {
    return hash((byte *) props, size * sizeof(Property));
}

Persistence::Persistence(const char *_sign, Property *_props_runtime, uint8_t props_size, int8_t *_mappings, uint8_t msz): given_sign_chr(_sign), props(_props_runtime), size(props_size), mappings(_mappings), mappings_size(msz) {}

void  Persistence::begin(){
    checkFactoryReset(props);
    EEPROM.get(BASE, signature);
    signature[SIGNATURE_SIZE - 1] = '\0';
    writeLog('I', ORIGIN, 100, isprint(signature[0]) ? signature : "___");
    unsigned long eeprom_hash;
    EEPROM.get(HASH_OFFSET, eeprom_hash);
    writeLog('I', ORIGIN, 101, eeprom_hash);
    if (strcmp(given_sign_chr, signature) != 0) {
        strcpy(signature, given_sign_chr);
        writeLog('W', ORIGIN, 501, signature);
        EEPROM.put(BASE, signature);
        storeProperties(props);
        storeMappings(mappings);
    } else if (eeprom_hash != hashProp(props, size)) {
        writeLog('W', ORIGIN, 301);
        loadProperties(props);
        loadMappings(mappings);
    }
}

void Persistence::storeMappings(int8_t *mappings) {
    for (uint8_t i = 0; i < mappings_size; i++) {
        EEPROM.put(MAPPINGS_OFFSET + i, mappings[i]);

    }
}

void Persistence::loadMappings(int8_t *mappings) {
    for (uint8_t i = 0; i < mappings_size; i++) {
        mappings[i] = (i < MAPPINGS_SIZE) ? (int8_t) EEPROM.read(MAPPINGS_OFFSET + i) :  -1;
    }
}

void Persistence::storeProperties(Property *props) {
    for (uint8_t i = 0; i < size; i++) {
        storeValue(i, props[i].runtime);
    }
    unsigned long hash = hashProp(props, size);
    writeLog('I', ORIGIN, 201, size);
    writeLog('I', ORIGIN, 101, hash);
    EEPROM.put(HASH_OFFSET, hash);
}

void Persistence::storeValue(uint8_t i, long val) {
    if (i <= size) {
        EEPROM.put(ADDR(i), val);
    }
}

void Persistence::loadProperties(Property *props) {
    for (uint8_t i = 0; i < size; i++) {
        EEPROM.get(ADDR(i), props[i].runtime);
    }
    writeLog('I', ORIGIN, 202, size);
}

void Persistence::checkFactoryReset(Property *props) {
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        writeLog('I', ORIGIN, 200);
        storeProperties(props);
        EEPROM.put(STATES_OFFSET, 0); // Clear state DISARM flags
    }
}

bool Persistence::loadState(uint8_t id) {
    return id < 8 ? (bool) ((EEPROM.read(STATES_OFFSET) & (1 << id)) >> id) : false;
}

void Persistence::storeState(uint8_t id, bool state) {
    if (id < 8) {
        EEPROM.put(STATES_OFFSET,
                   state ? EEPROM.read(STATES_OFFSET) | (1 << id) :  EEPROM.read(STATES_OFFSET) & (~(1 << id)));
    }
}
