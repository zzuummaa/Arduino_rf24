//
// Created by Stephan on 24.11.2018.
//

#ifndef ARDUINO_RF24_DEFS_H
#define ARDUINO_RF24_DEFS_H

#define SERIAL_TX_BUFFER_SIZE 128
#define SERIAL_RX_BUFFER_SIZE 128
#include "Arduino.h"
#include "printf.h"

#define ENABLE_DEBUG 0

#define printf_L(fmt, args...) if (ENABLE_DEBUG == 1) printf(fmt, ## args)

#endif //ARDUINO_RF24_DEFS_H
