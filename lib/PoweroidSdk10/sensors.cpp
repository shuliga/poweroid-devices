//
// Created by SHL on 20.03.2017.
//

#include "pin_io.h"
#include "commons.h"
#include "timings.h"
#include "sensors.h"

const long INST_DELAY = 1000L;

static TimingState flash_333 = TimingState(333);
static TimingState hold_on[3] = {INST_DELAY, INST_DELAY, INST_DELAY};

DHT *Sensors::searchDht(){
    DHT *result = NULL;
    for(uint8_t i = 0 ; i < ARRAY_SIZE(IN_PINS); i++){
        result = new DHT(IN_PINS[i], DHTTYPE);
        result->begin();
        float val = result->readHumidity();
        if (!isnan(val)) {
            Serial.print(F("DHT installed on pin:"));
            Serial.println(IN_PINS[i]);
            return result;
        } else {
            delete result;
        }
    }
    return NULL;
}

void Sensors::updateTnH() {
    if (dht != NULL && ping(&pollTiming)) {
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

bool Sensors::checkInstalled(uint8_t pin, bool inst) {
    bool sign = readPinLow(pin);
    if (!inst && sign) {
        Serial.print(F("Sensor installed on pin "));
        Serial.println(pin);
    }
    return inst || sign;
}

bool Sensors::checkInstalledWithDelay(uint8_t pin, bool inst, TimingState *hold_on) {
    bool sign = isTimeAfter(hold_on, readPinLow(pin));
    if (!inst && sign) {
        Serial.print(F("Sensor installed on pin "));
        Serial.println(pin);
    }
    return inst || sign;
}

void Sensors::init_sensors() {
    dht = searchDht();
    for (uint8_t i = 0; i < 3; i++) {
        hold_on[i].interval = INST_DELAY;
    }
}

void Sensors::process(){
    check_installed();
    updateTnH();
}

void Sensors::check_installed() {
    for (uint8_t i = 0; i < 3; i++) {
        installed[i] = checkInstalledWithDelay(IN_PINS[i], installed[i], &hold_on[i]);
    }
}

bool Sensors::is_dht_installed() {
    return dht != NULL;
}


bool Sensors::is_sensor_on(uint8_t index) {
    return readPinLow(IN_PINS[index]) && installed[index];
}

int Sensors::get_sensor_val(uint8_t index) {
    if (installed[index]) {
        return readPinVal(INA_PINS[index]);
    } else {
        return -1;
    }
}

bool Sensors::is_sensor_val(uint8_t index, uint8_t val) {
    return get_sensor_val(index) == val;
}

char *Sensors::printSensor(boolean sensors[], uint8_t idx) {
    char *result = new char;
    sprintf(result, "Sensor[%i]: %s", idx, sensors[idx] ? "installed" : "not installed");
    return result;
}

