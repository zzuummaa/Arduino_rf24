//
// Created by Stephan on 23.11.2018.
//

#ifndef ARDUINO_RF24_HEX_STR_H
#define ARDUINO_RF24_HEX_STR_H

#include <avr/io.h>

constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

char* hexStr(uint8_t *data, int len, char* str) {
    for (int i = 0; i < len; ++i) {
        str[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
        str[2 * i + 1] = hexmap[data[i] & 0x0F];
    }
    str[2 * len] = '\0';
    return str;
}

#endif //ARDUINO_RF24_HEX_STR_H
