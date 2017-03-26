//
// Created by SHL on 20.03.2017.
//

#ifndef _SENSORS_H
#define _SENSORS_H

#include "commons.h"
#include "timings.h"
#include <DHT.h>

#define DHTTYPE DHT22

extern const uint8_t IN_PINS[];
extern const uint8_t INA_PINS[];

class Sensors {
public:

    void updateTnH();

    float getTemperature() const;

    float getHumidity() const;

    void process();

    bool checkInstalled(uint8_t pin, bool inst);

    bool checkInstalledWithDelay(uint8_t pin, bool inst, TimingState &hold_on);

    void init_sensors();

    void check_installed();

    bool is_dht_installed();

    bool is_sensor_on(uint8_t index);

    int get_sensor_val(uint8_t index);

    bool is_sensor_val(uint8_t index, uint8_t val);

    char *printSensor(uint8_t idx);

    uint8_t size();

    void printDht(char *buffer);

private:
    TimingState pollTiming = TimingState(5000L);
    float temp;
    float humid;

    DHT *dht;

    DHT *searchDht();
};


#endif //_SENSORS_H
