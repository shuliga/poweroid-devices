//
// Created by SHL on 28.09.2019.
//

#ifndef DATETIME_H
#define DATETIME_H

#include <avr/io.h>

extern const char * TIME_FMT;
extern const char * DATE_FMT;

class DateTime{
public:
    DateTime();

    void setTimeFromString(const char time[9]); // Time string format: "hh:mm:ss"
    void setDateFromString(const char date[9]); // Date string format: "dd/MM/yy"
    void getTimeString(char *time);
    void getDateString(char *date);
    uint8_t getDoW();

    void setDoW(uint8_t dow);
};

#endif //DATETIME_H
