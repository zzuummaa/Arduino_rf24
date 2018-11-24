//
// Created by Stephan on 24.11.2018.
//

#ifndef ARDUINO_RF24_DEFS_H
#define ARDUINO_RF24_DEFS_H

#define ENABLE_DEBUG 0

#if ENABLE_DEBUG == 1
    #include "printf.h"
    #define printf_L(f_, ...) printf((f_), ##__VA_ARGS__)
#elif ENABLE_DEBUG == 0
    inline void printf_moc(const char *__fmt, ...) {};
    inline void printf_begin(void) {}
    #define printf_L(f_, ...) printf_moc((f_), ##__VA_ARGS__)
#endif

#endif //ARDUINO_RF24_DEFS_H
