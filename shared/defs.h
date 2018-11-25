//
// Created by Stephan on 24.11.2018.
//

#ifndef ARDUINO_RF24_DEFS_H
#define ARDUINO_RF24_DEFS_H

#include <printf.h>

#define ENABLE_DEBUG 1

#if ENABLE_DEBUG == 1
    #define printf_L(fmt, args...) printf(fmt, ## args)
#elif ENABLE_DEBUG == 0
    inline int printf_moc(const char *__fmt, ...) {};
    #define printf_L(f_, ...) printf_moc((f_), ##__VA_ARGS__)
#endif

#endif //ARDUINO_RF24_DEFS_H
