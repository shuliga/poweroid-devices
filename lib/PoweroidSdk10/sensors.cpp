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

Sensors::Sensors(){
#ifdef  DHTPIN
    dht = new DHT(DHTPIN, DHTTYPE);
    dht->begin();
#endif
}

void Sensors::updateTnH() {
    if (dht != NULL && ping(&pollTiming)){
        temp = dht->readTemperature();
        humid = dht->readHumidity();
    }
}

float Sensors::getTemperature()const {
    return temp;
}

float Sensors::getHumidity()const {
    return humid;
}

bool Sensors::checkInstalled(int pin, bool inst){
    bool sign = readPinLow(pin);
    if (!inst && sign) {
        Serial.print(F("Sensor installed on pin "));
        Serial.println(pin);
    }
    return inst || sign;
}

bool Sensors::checkInstalledWithDelay(int pin, bool inst, TimingState *hold_on){
    bool sign = isTimeAfter(hold_on, readPinLow(pin));
    if (!inst && sign) {
        Serial.print(F("Sensor installed on pin "));
        Serial.println(pin);
    }
    return inst || sign;
}

void Sensors::init_sensors()const {
    for(int i=0; i < 3; i++){
        hold_on[i].interval = INST_DELAY;
    }
}

void Sensors::check_installed(){
    for(int i = 0; i < 3; i++){
        installed[i] = checkInstalledWithDelay(IN_PINS[i], installed[i], &hold_on[i]);
    }
}

bool Sensors::is_sensor_on(int index){
    return readPinLow(IN_PINS[index]) && installed[index];
}

int Sensors::get_sensor_val(int index){
    if (installed[index]) {
        return readPinVal(INA_PINS[index]);
    } else {
        return -1;
    }
}

bool Sensors::is_sensor_val(int index, int val){
    return get_sensor_val(index) == val;
}
