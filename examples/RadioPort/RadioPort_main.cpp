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
Role role;                                           // The role of the current running sketch

uint8_t buff[500];
uint8_t pingInBuff[sizeof(buff)+1];
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
        role = Role::TRANSMITTER;
    else
        role = Role::RECEIVER;

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

int readline(char *buffer, int len, bool * isEndLine = nullptr);

inline void transmitLogic() {
    int serialBytesLen = readline((char*)buff, sizeof(buff));
    if (serialBytesLen <= 0) return;

    if (sizeof(buff) > serialBytesLen + 2) {
        buff[serialBytesLen] = 0;
        printf("'%s'\n\r", (char*)buff);
        buff[serialBytesLen] = '\n';
        buff[serialBytesLen+1] = '\r';
        buff[serialBytesLen+2] = 0;
        serialBytesLen += 2;
    } else {
        return;
    }

    printf("Write %d bytes...\n\r", serialBytesLen);
    curTime = micros();
    size_t writenLen = radioPort.transmit(buff, (size_t)serialBytesLen);
    if (writenLen > 0) {
#if ENABLE_DEBUG
        printf("Writen %d bytes, time: %lu ms\r\n", writenLen, (micros() - curTime) / 1000);
#endif
    }

    if (writenLen != sizeof(buff)) delay(radioPort.getTimeout() + 1);
}

inline void receiveLogic() {
    memset(pingInBuff, 0, sizeof(pingInBuff));
    curTime = micros();
    size_t readLen = radioPort.receive(pingInBuff, sizeof(pingInBuff)-1);
    if (readLen > 0) {
#if ENABLE_DEBUG
        printf("Read %d valid bytes, time: %lu ms\n\r", readLen, (micros() - curTime) / 1000);
#endif
        Serial.print((char*)pingInBuff);
        delay(radioPort.getTimeout());
    }

}

void loop() {
    if (role == Role::TRANSMITTER) {
        transmitLogic();
    }

    if (role == Role::RECEIVER) {
        receiveLogic();
    }
}

inline int readline(char *buffer, int len, bool * isEndLine) {
    int ch;
    char* p = buffer;
    if (isEndLine != nullptr) *isEndLine = false;

    while (p - buffer < len) {
        ch = Serial.read();
        if (ch == -1) continue;
        if (ch != '\r' && ch != '\n') {
            Serial.print((char)ch);
            *p++ = (char)ch;
        }

        if (ch == '\n' || ch == '\r') {
            int tmp = Serial.peek();
            if (tmp == '\n' || tmp == '\r') Serial.read();
            Serial.println();
            if (isEndLine != nullptr) *isEndLine = true;
            printf("p - buffer = %d\n\r", (int)(p - buffer));
            return (int)(p - buffer);
        }
    }

    return len;
}