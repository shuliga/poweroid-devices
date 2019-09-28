//
// Created by SHL on 28.09.2019.
//

#include "datetime.h"
#include <DS1307/DS1307.h>

const char * TIME_FMT = "%02d:%02d:%02d";
const char * DATE_FMT = "%02d/%02d/%02d";

DateTime::DateTime() {
    RTC.start();
}

void DateTime::setTimeFromString(const char time[9]) {

    char tmp[3];

    strncpy(tmp, time, 2);
    uint8_t hr = atoi(tmp);

    strncpy(tmp, time + 3, 2);
    uint8_t min = atoi(tmp);

    strncpy(tmp, time + 6, 2);
    uint8_t sec = atoi(tmp);

    RTC.set(DS1307_HR, hr);
    RTC.set(DS1307_MIN, min);
    RTC.set(DS1307_SEC, sec);
}

void DateTime::setDateFromString(const char date[9]) {
    char tmp[3];

    strncpy(tmp, date, 2);
    uint8_t day = atoi(tmp);

    strncpy(tmp, date + 3, 2);
    uint8_t month = atoi(tmp);

    strncpy(tmp, date + 6, 2);
    uint8_t year = atoi(tmp);

    RTC.set(DS1307_DATE, day);
    RTC.set(DS1307_MTH, month);
    RTC.set(DS1307_YR, year);
};

void DateTime::getTimeString(char *time) {
    sprintf(time, TIME_FMT, RTC.get(DS1307_HR, true), RTC.get(DS1307_MIN, true), RTC.get(DS1307_SEC, true));
}

void DateTime::getDateString(char *date) {
    sprintf(date, TIME_FMT, RTC.get(DS1307_DATE, true), RTC.get(DS1307_MTH, true), RTC.get(DS1307_YR, true) - DS1307_BASE_YR);
}

uint8_t DateTime::getDoW() {
    return RTC.get(DS1307_DOW, true);
}

void DateTime::setDoW(uint8_t dow) {
    return RTC.set(DS1307_DOW, dow);
}
