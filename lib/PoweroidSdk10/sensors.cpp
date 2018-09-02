//
// Created by SHL on 20.03.2017.
//

#include "global.h"
#include "sensors.h"
#include "commands.h"

// Console output codes
// 100 -
// 200 - DHT installed (pin)
// 201 - Sensor installed (pin)
// 500 - DHT not installed
// 501 - Sensor not installed (pin)


const long INST_DELAY = 50L;

const uint8_t IN_PINS[] = {IN1_PIN, IN2_PIN, IN3_PIN};
const uint8_t INA_PINS[] = {INA1_PIN, INA2_PIN, INA3_PIN};

static TimingState flash_333 = TimingState(333);
static TimingState pollTiming = TimingState(5000L);

static TimingState hold_on[3] = {INST_DELAY, INST_DELAY, INST_DELAY};
static bool installed[ARRAY_SIZE(IN_PINS)];
static bool dht_installed;
static bool propagate = false;
static bool dht_set = false;

Sensors::Sensors() : dht(DHT_PIN, DHTTYPE) {
}

void Sensors::searchDht() {
    dht.begin();
    float val = dht.readHumidity();
    if (!isnan(val)) {
#ifdef DEBUG
        writeLog('I', ORIGIN, 200, DHT_PIN);
#endif
        for(int i = 0; i < ARRAY_SIZE(IN_PINS); i++){
            if (IN_PINS[i] == DHT_PIN){
                installed[i] = true;
            }
        }
        dht_installed = true;
    }
}

void Sensors::updateTnH() {
    if (dht_installed && pollTiming.ping()) {
        temp = dht.readTemperature();
        humid = dht.readHumidity();
        if (propagate) {
            sprintf(BUFF, ":%c%c", getInt(temp), getInt(humid));
            printCmd(cu.cmd_str.CMD_SET_DHT, BUFF);
        }
    }
}

float Sensors::getTemperature() const {
    return temp;
}

float Sensors::getHumidity() const {
    return humid;
}

void Sensors::printInstalled(uint8_t pin) {
    writeLog('I', ORIGIN, 201, pin);
}

bool Sensors::checkInstalled(uint8_t pin, bool inst) {
    bool sign = digitalRead(pin) == LOW;
    if (!inst && sign) {
        printInstalled(pin);
    }
    return inst || sign;
}

bool Sensors::checkInstalledWithDelay(uint8_t pin, bool inst, TimingState &hold_on) {
    bool sign = false;
    if (!inst) {
        sign = hold_on.isTimeAfter(digitalRead(pin) == LOW);
        if (sign) {
            printInstalled(pin);
        }
    }
    return inst || sign;
}

void Sensors::initSensors(bool _propagate) {
    propagate = _propagate;
    delay(1000L);
    searchDht();
    pollTiming.reset();
}

void Sensors::process() {
    checkInstalled();
    updateTnH();
}

void Sensors::checkInstalled() {
    for (uint8_t i = 0; i < ARRAY_SIZE(IN_PINS); i++) {
        installed[i] = checkInstalled(IN_PINS[i], installed[i]);
    }
}

bool Sensors::isDhtInstalled() {
    return dht_installed;
}


bool Sensors::isSensorOn(uint8_t index) {
    return hold_on[index].isTimeAfter(digitalRead(IN_PINS[index]) == LOW && installed[index]);
}

int Sensors::getSensorVal(uint8_t index) {
    if (installed[index]) {
        return analogRead(INA_PINS[index]);
    } else {
        return -1;
    }
}

bool Sensors::isSensorVal(uint8_t index, uint8_t val) {
    return getSensorVal(index) == val;
}

const char *Sensors::printSensor(uint8_t idx) {
    writeLog('I', ORIGIN, installed[idx] ? 201 : 501, idx);
}

uint8_t Sensors::size() {
    return ARRAY_SIZE(IN_PINS);
}

const char *Sensors::printDht() {
    if (dht_installed || dht_set) {
        sprintf(BUFF, "%i~C, %i%%", getInt(temp), getInt(humid));
    } else {
        noInfoToBuff();
    }
    return BUFF;
}

int8_t Sensors::getInt(float f) const { return (int8_t) floor(f + 0.5); }

void Sensors::setDht(int8_t _temp, uint8_t _humid) {
    if (!dht_installed) {
        temp = _temp;
        humid = _humid;
        dht_set = true;
    }
}

