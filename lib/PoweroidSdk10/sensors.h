//
// Created by SHL on 20.03.2017.
//

#ifndef _SENSORS_H
#define _SENSORS_H

#include <DHT.h>
#include "timings.h"

#define DHTPIN 5
#define DHTTYPE DHT22

static bool installed[3];

class Sensors {
public:
    Sensors();
    void updateTnH();
    float getTemperature();
    float getHumidity();

private:
    TimingState timing = {0, 5000L, 0, 0, 0, 0};
    float temp;
    float humid;

    DHT *dht;
};


#endif //_SENSORS_H
