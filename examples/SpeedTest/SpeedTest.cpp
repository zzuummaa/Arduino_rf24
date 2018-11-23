//
// Created by Stephan on 23.11.2018.
//

/*
  // March 2014 - TMRh20 - Updated along with High Speed RF24 Library fork
  // Parts derived from examples by J. Coliz <maniacbug@ymail.com>
*/
/**
 * Example for efficient call-response using ack-payloads
 *
 * This example continues to make use of all the normal functionality of the radios including
 * the auto-ack and auto-retry features, but allows ack-payloads to be written optionally as well.
 * This allows very fast call-response communication, with the responding radio never having to
 * switch out of Primary Receiver mode to send back a payload, but having the option to if wanting
 * to initiate communication instead of respond to a commmunication.
 */



#include <Arduino.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "hex_str.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio(7, 8);

// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter
const int role_pin = 5;

// Topology
const uint64_t pipes[2] = {0xABCDABCD71LL,
                           0x544d52687CLL};              // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.

typedef enum {
    role_ping_out = 1, role_pong_back
} role_e;                 // The various roles supported by this sketch
const char *role_friendly_name[] = {"invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

char spaces[8];

unsigned long serialTime = 0;

// A single byte to keep track of the data being sent back and forth
uint8_t counter[32];
char strBuf[sizeof(counter)*2 + 1];
uint32_t successCount = 0;
uint32_t failCount = 0;

inline uint8_t* inc(uint8_t* p, int len) {
    uint8_t* p_tmp = p;

    for (int i = 0; i < len; ++i) {
        (*p)++;
        if (*p != 0) {
            break;
        }
        p++;
    }

    return p_tmp;
}

void setup() {
    memset(spaces, ' ', sizeof(spaces)-1); spaces[sizeof(spaces)-1] = '\0';
    memset(counter, 0, sizeof(counter));

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
    Serial.print(F("\n\rRF24/examples/SpeedTest/\n\rROLE: "));
    Serial.println(role_friendly_name[role]);

    // Setup and configure rf radio

    radio.begin();
    radio.setAutoAck(true);
    radio.enableAckPayload();               // Allow optional ack payloads
    radio.setRetries(5, 15);                 // Smallest time between retries, max no. of retries
    radio.enableDynamicPayloads();
    if (role == role_ping_out) {
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1, pipes[1]);
    } else {
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(1, pipes[0]);
    }
    radio.setDataRate(RF24_2MBPS);
    radio.startListening();                 // Start listening
    radio.printDetails();                   // Dump the configuration of the rf unit for debugging
    Serial.println();
}

void performTest(int payloadLen, unsigned long duration_ms) {
    printf("\r\nTesting payload %d bytes...\r\n", payloadLen);

    unsigned long duration_mcs = duration_ms * (unsigned long)1000;
    memset(counter, 0, sizeof(counter));
    do {
        unsigned long time = micros();

        radio.stopListening();                                  // First, stop listening so we can talk.
//        printf("Now sending %s as payload. ", hexStr(counter, sizeof(counter), strBuf));
                               // Take the time, and send it.  This will block until complete
        //Called when STANDBY-I mode is engaged (User is finished sending)
        if (!radio.write(counter, (uint8_t)payloadLen)) {
            failCount++;
//            printf("failed. sucs/fail: %d/%d%s\r\n", successCount, failCount, spaces);
        } else {
            if (!radio.isAckPayloadAvailable()) {
                failCount++;
                unsigned long tim = micros();
//                printf("Blank Payload Received. Delay: %1u microsec. sucs/fail: %d/%d.%s\r\n", tim - time, successCount, failCount, spaces);
            } else {
                unsigned long tim = micros();
                byte ackPayload;
                radio.read(&ackPayload, sizeof(ackPayload));
                if (ackPayload == 0xFF) {
                    successCount++;
                    inc(counter, sizeof(counter));
                } else {
                    failCount++;
                }
//                printf("Got response %s, delay: %lu microsec. sucs/fail: %d/%d\r\n",
//                        hexStr(gotBuff, sizeof(gotBuff), strBuf), tim - time, successCount, failCount);

            }
        }
        serialTime += micros() - time;

    } while (serialTime < duration_mcs);


    printf("sucs/fail: %lu/%lu, speed: %lu bytes/sec, period: %lu\r\n", successCount, failCount, successCount * payloadLen, serialTime);
    successCount = 0;
    serialTime = 0;
    failCount = 0;
}

void loop(void) {

    if (role == role_ping_out) {
        for (int i = 32; i >= 2; i -= 4) {
            performTest(i, 6000);
        }
        Serial.println("=======================");
        Serial.println("Tests is end");
        delay(30 * 1000);
    }

    // Pong back role.  Receive each packet, dump it out, and send it back

    if (role == role_pong_back) {
        byte pipeNo;                    // Dump the payloads until we've gotten everything
        byte ackPayload = 0xFF;
        while (radio.available(&pipeNo)) {
            uint8_t len = radio.getDynamicPayloadSize();
            if (len == 0) continue;
            memset(counter, 0, sizeof(counter));
            radio.read(counter, len);
            radio.writeAckPayload(pipeNo, &ackPayload, sizeof(ackPayload));
        }
    }
}
