//
// Created by SHL on 20.03.2017.
//

#include "sensors.h"
#include "timings.h"

Sensors::Sensors(){
#ifdef  DHTPIN
    dht = new DHT(DHTPIN, DHTTYPE);
    dht->begin();
#endif
}

void Sensors::updateTnH() {
    if (dht != NULL && ping(&timing)){
        temp = dht->readTemperature();
        humid = dht->readHumidity();
    }
}

float Sensors::getTemperature() {
    return temp;
}

float Sensors::getHumidity() {
    return humid;
}

