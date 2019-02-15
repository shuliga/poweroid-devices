//
// Created by SHL on 15.02.2019.
//

#ifndef TIMER_BUTTON_1X1_PRO_ULTRASONIC_H
#define TIMER_BUTTON_1X1_PRO_ULTRASONIC_H

#include "commons.h"
#include "sensors.h"

#define US_TRIGGER_PIN INA3_PIN
#define US_ECHO_PIN IN3_PIN

class Ultrasonic{
public:
    void begin();
    uint16_t getDistance();
private:
};

extern Ultrasonic ULTRASONIC;

#endif //TIMER_BUTTON_1X1_PRO_ULTRASONIC_H
