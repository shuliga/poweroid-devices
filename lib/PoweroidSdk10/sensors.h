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
    float getTemperature()const;
    float getHumidity()const;

    bool checkInstalled(uint8_t pin, bool inst);
    bool checkInstalledWithDelay(uint8_t pin, bool inst, TimingState *hold_on);
    void init_sensors()const;
    void check_installed();
    bool is_sensor_on(uint8_t index);
    int get_sensor_val(uint8_t index);
    bool is_sensor_val(uint8_t index, uint8_t val);
    char * printSensor(boolean sensors[], uint8_t idx);

private:
    TimingState pollTiming = TimingState(5000L);
    float temp;
    float humid;

    DHT *dht;
};


#endif //_SENSORS_H
