//
// Created by SHL on 20.03.2017.
//

#include "pin_io.h"
#include "commons.h"
#include "timings.h"
#include "sensors.h"

const long INST_DELAY = 50L;

const uint8_t IN_PINS[] = {IN1_PIN, IN2_PIN, IN3_PIN};
const uint8_t INA_PINS[] = {INA1_PIN, INA2_PIN, INA3_PIN};

static TimingState flash_333 = TimingState(333);
static TimingState hold_on[3] = {INST_DELAY, INST_DELAY, INST_DELAY};
static bool installed[ARRAY_SIZE(IN_PINS)];
static char CHAR_BUF[32];

DHT *Sensors::searchDht() {
    DHT *result = NULL;
    for (uint8_t i = 0; i < ARRAY_SIZE(IN_PINS); i++) {
        result = new DHT(IN_PINS[i], DHTTYPE);
        result->begin();
        float val = result->readHumidity();
        if (!isnan(val)) {
            Serial.print(F("DHT installed on pin:"));
            Serial.println(IN_PINS[i]);
            installed[i] = true;
            return result;
        } else {
            pinMode(IN_PINS[i], INPUT_PULLUP);
//            digitalWrite(IN_PINS[i], HIGH);
            delete result;
        }
    }
    return NULL;
}

void Sensors::updateTnH() {
    if (isDhtInstalled() && pollTiming.ping()) {
        temp = dht->readTemperature();
        humid = dht->readHumidity();
    }
}

float Sensors::getTemperature() const {
    return temp;
}

float Sensors::getHumidity() const {
    return humid;
}

void Sensors::printInstalled(uint8_t pin) {
    Serial.print(F("Sensor installed on pin "));
    Serial.println(pin);
}

bool Sensors::checkInstalled(uint8_t pin, bool inst) {
    bool sign = readPinLow(pin);
    if (!inst && sign) {
        printInstalled(pin);
    }
    return inst || sign;
}

bool Sensors::checkInstalledWithDelay(uint8_t pin, bool inst, TimingState &hold_on) {
    bool sign = false;
    if (!inst) {
        sign = hold_on.isTimeAfter(readPinLow(pin));
        if (sign) {
            printInstalled(pin);
        }
    }
    return inst || sign;
}

void Sensors::initSensors() {
    dht = searchDht();
    for (uint8_t i = 0; i < ARRAY_SIZE(IN_PINS); i++) {
//        hold_on[i].interval = INST_DELAY;
    }
}

void Sensors::process() {
    checkInstalled();
    updateTnH();
}

void Sensors::checkInstalled() {
    for (uint8_t i = 0; i < ARRAY_SIZE(IN_PINS); i++) {
        installed[i] = checkInstalled(IN_PINS[i], installed[i]);
//        installed[i] = checkInstalledWithDelay(IN_PINS[i], installed[i], hold_on[i]);
    }
}

bool Sensors::isDhtInstalled() {
    return dht != NULL;
}


bool Sensors::isSensorOn(uint8_t index) {
    return hold_on[index].isTimeAfter(readPinLow(IN_PINS[index]) && installed[index]);
}

int Sensors::getSensorVal(uint8_t index) {
    if (installed[index]) {
        return readPinVal(INA_PINS[index]);
    } else {
        return -1;
    }
}

bool Sensors::isSensorVal(uint8_t index, uint8_t val) {
    return getSensorVal(index) == val;
}

const char *Sensors::printSensor(uint8_t idx) {
    sprintf(CHAR_BUF, "Sensor[%i]: %s", idx, installed[idx] ? "installed" : "not installed");
    return CHAR_BUF;
}

uint8_t Sensors::size() {
    return ARRAY_SIZE(IN_PINS);
}

const char *Sensors::printDht() {
    sprintf(CHAR_BUF, "%i~C, %i%%", (int) floor(temp + 0.5), (int) floor(humid + 0.5));
    return CHAR_BUF;
}

