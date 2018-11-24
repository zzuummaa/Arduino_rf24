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

RadioPort radioPort;

// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter
const int role_pin = 5;

const char *role_friendly_name[] = {"invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

uint8_t buff[200];
uint8_t pingInBuff[sizeof(buff)];
char strBuf[sizeof(buff)*2 + 1];

void setup() {
    for (int i = 0; i < sizeof(buff); ++i) {
        buff[i] = (uint8_t)(i % UINT8_MAX);
    }

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
    Serial.print(F("\n\rRF24/examples/RadioPort/\n\rROLE: "));
    Serial.println(role_friendly_name[role]);

    radioPort.setTimeout(1000lu);
    radioPort.begin(role);

    if (!radioPort.isChipConnected()) {
        printf("RF24 isn't connected to SPI\n\r");
    } else {
        printf("RF24 connected to SPI\n\r");
    }
    while (!radioPort.isChipConnected());

    radioPort.printDetails();
    Serial.println();
}

void loop() {

    if (role == role_ping_out) {
        int writenLen = radioPort.transmit(buff, sizeof(buff));
        if (writenLen > 0) printf("Writen %d bytes\r\n", writenLen);
        delay(radioPort.getTimeout() + 1);
    }

    if (role == role_pong_back) {
        memset(pingInBuff, 0, sizeof(pingInBuff));
        int readLen = radioPort.receive(pingInBuff, sizeof(pingInBuff));
        if (readLen > 0) {
            if (memcmp(pingInBuff, buff, readLen) == 0) {
                printf("Read %d valid bytes.\n\r", readLen);
            } else {
                printf("Read %d bytes\r\n", readLen);
            }
        }
        delay(radioPort.getTimeout() + 1);
    }
}