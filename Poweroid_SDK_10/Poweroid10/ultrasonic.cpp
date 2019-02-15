//
// Created by SHL on 15.02.2019.
//

#include "ultrasonic.h"

void Ultrasonic::begin(){
    pinMode(US_TRIGGER_PIN, OUTPUT);
    pinMode(US_ECHO_PIN, INPUT);

}

uint16_t Ultrasonic::getDistance(){
    delayMicroseconds(10);
    digitalWrite(US_TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(US_TRIGGER_PIN, LOW);
    long echo = pulseInLong(US_ECHO_PIN, HIGH);
    return echo / 56.5;
}

Ultrasonic ULTRASONIC;