//
// Created by Stephan on 23.11.2018.
//

#include "Arduino.h"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "RadioPort.h"
#include "hex_str.h"
#include "defs.h"

RadioPort radioPort;

// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter
const int role_pin = 5;

const char *role_friendly_name[] = {"invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

uint8_t buff[500];
uint8_t pingInBuff[sizeof(buff)];
char strBuf[sizeof(buff)*2 + 1];

unsigned long curTime;

void setup() {
    for (int i = 0; i < sizeof(buff); ++i) {
        buff[i] = (uint8_t)(i % UINT8_MAX);
    }

    // Noise reset resist
    pinMode(10, OUTPUT);

    pinMode(role_pin, INPUT);
    digitalWrite(role_pin, HIGH);
    delay(20); // Just to get a solid reading on the role pin

    // read the address pin, establish our role
    if (digitalRead(role_pin))
        role = role_ping_out;
    else
        role = role_pong_back;

    Serial.begin(115200);
    printf_begin();
#if ENABLE_DEBUG
    Serial.print(F("\n\rRF24/examples/RadioPort/\n\rROLE: "));
    Serial.println(role_friendly_name[role]);
#endif
    radioPort.setTimeout(500lu);
    radioPort.begin(role);

#if ENABLE_DEBUG
    if (!radioPort.isChipConnected()) {
        printf("RF24 isn't connected to SPI\n\r");
    } else {
        printf("RF24 connected to SPI\n\r");
    }
#endif
    while (!radioPort.isChipConnected());

#if ENABLE_DEBUG
    radioPort.printDetails();
    Serial.println();
#endif
}

void loop() {
    curTime = micros();

    if (role == role_ping_out) {
        int writenLen = radioPort.transmit(buff, sizeof(buff));
        if (writenLen > 0) {
#if ENABLE_DEBUG
            printf("Writen %d bytes, time: %lu ms\r\n", writenLen, (micros() - curTime) / 1000);
#endif
        }

        if (writenLen != sizeof(buff)) delay(radioPort.getTimeout() + 1);
    }

    if (role == role_pong_back) {
        memset(pingInBuff, 0, sizeof(pingInBuff));
        int readLen = radioPort.receive(pingInBuff, sizeof(pingInBuff));
        if (readLen > 0) {
            if (memcmp(pingInBuff, buff, (size_t)readLen) == 0) {
#if ENABLE_DEBUG
                printf("Read %d valid bytes, time: %lu ms\n\r", readLen, (micros() - curTime) / 1000);
#endif
            } else {
#if ENABLE_DEBUG
                printf("Read %d bytes, time: %lu ms\r\n", readLen, (micros() - curTime) / 1000);
#endif
            }
        }
        if (readLen != sizeof(pingInBuff)) delay(radioPort.getTimeout() + 1);
    }
}