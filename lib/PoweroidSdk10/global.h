//
// Created by SHL on 18.11.2017.
//

#ifndef POWEROID_10_GLOBAL_H_H
#define POWEROID_10_GLOBAL_H_H

#ifdef COMMONS_H
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define BUFF_SIZE 65

EXTERN char BUFF[BUFF_SIZE];

#endif //POWEROID_10_GLOBAL_H_H
