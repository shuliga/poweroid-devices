
#include <EEPROM.h>
#include "pin_io.h"
#include "commons.h"
#include "persistence.h"

// Console output codes
// 101 - Factory reset
// 201 - Properties stored in EEPROM.
// 202 - Properties loaded from EEPROM
// 501 - Not a native EEPROM. Overwriting signature.
// 502 - Hash differs from factory.


unsigned long hashProp(long *props, int size) {
    return hash((byte *) props, size * sizeof(long));
}

Persistence::Persistence(String s, long *props_runtime, uint8_t props_size, uint8_t *mappings, uint8_t msz) {
    delay(501);
    checkFactoryReset(props_runtime);
    size = props_size;
    mappings_size = msz;
    EEPROM.get(BASE, signature);
    String sign = String(signature);
    unsigned long eeprom_hash;
    EEPROM.get(HASH_OFFSET, eeprom_hash);
    Serial.print(F("EEPROM signature '"));
    Serial.print(sign + "', hash:");
    Serial.println(eeprom_hash);
    if (sign != s) {
        writeLog('W', ORIGIN, 510);
        Serial.println("Overwriting signature '" + sign + "' with: '" + s + "'");
        s.toCharArray(signature, SIGNATURE_SIZE);
        EEPROM.put(BASE, signature);
        storeProperties(props_runtime);
        storeMappings(mappings);
        return;
    }
    if (eeprom_hash != hashProp(props_runtime, size)) {
        writeLog('W', ORIGIN, 502);
        loadProperties(props_runtime);
        loadMappings(mappings);
    }
}

void Persistence::storeMappings(uint8_t *mappings) {
    for (uint8_t i = 0; i < mappings_size; i++) {
        storeMapping(i, mappings[i]);
    }
}

void Persistence::loadMappings(uint8_t *mappings) {
    for (uint8_t i = 0; i < mappings_size; i++) {
        mappings[i] = (uint8_t) loadMapping(i);
    }
}

void Persistence::storeProperties(long *props) {
    for (uint8_t i = 0; i < size; i++) {
        storeValue(i, props[i]);
    }
    unsigned long hash = hashProp(props, size);
    char c_hash[11];
    c_hash[10] = 0;
    ultoa(hash, c_hash, 10);
    writeLog('I', ORIGIN, 201, size);
    Serial.println("Hash: " + String(c_hash));
    EEPROM.put(HASH_OFFSET, hash);
}

void Persistence::storeValue(uint8_t i, long val) {
    if (i <= size) {
        EEPROM.put(ADDR(i), val);
    }
}

void Persistence::loadProperties(long *props) {
    for (uint8_t i = 0; i < size; i++) {
        EEPROM.get(ADDR(i), props[i]);
    }
    writeLog('I', ORIGIN, 202, size);
}

void Persistence::checkFactoryReset(long *props_runtime) {
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        storeProperties(props_runtime);
        EEPROM.put(STATES_OFFSET, 0); // Clear state DISARM flags
        writeLog('I', ORIGIN, 101);
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

int8_t Persistence::loadMapping(uint8_t id) {
    return (id < MAPPINGS_SIZE) ? (int8_t) EEPROM.read(MAPPINGS_OFFSET + id) :  -1;
}

void Persistence::storeMapping(uint8_t id, int8_t mapped_id) {
    if (id < MAPPINGS_SIZE) {
        EEPROM.put(MAPPINGS_OFFSET + id, mapped_id);
    }
}
