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
uint8_t counter[4];
uint8_t gotBuff[sizeof(counter)];
char strBuf[sizeof(counter)*2 + 1];
int successCount = 0;
int failCount = 0;

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
    Serial.print(F("\n\rRF24/examples/pingpair_ack/\n\rROLE: "));
    Serial.println(role_friendly_name[role]);
    Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

    // Setup and configure rf radio

    radio.begin();
    radio.setAutoAck(true);
    radio.enableAckPayload();               // Allow optional ack payloads
    radio.setRetries(0, 15);                 // Smallest time between retries, max no. of retries
    radio.setPayloadSize(sizeof(counter));                // Here we are sending 1-byte payloads to test the call-response speed
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

void loop(void) {

    if (role == role_ping_out) {

        radio.stopListening();                                  // First, stop listening so we can talk.

//        printf("Now sending %s as payload. ", hexStr(counter, sizeof(counter), strBuf));
        unsigned long time = micros();                          // Take the time, and send it.  This will block until complete
        //Called when STANDBY-I mode is engaged (User is finished sending)
        if (!radio.write(counter, sizeof(counter))) {
            failCount++;
//            printf("failed. sucs/fail: %d/%d%s\r\n", successCount, failCount, spaces);
        } else {
            if (!radio.available()) {
                failCount++;
                unsigned long tim = micros();
//                printf("Blank Payload Received. Delay: %1u microsec. sucs/fail: %d/%d.%s\r\n", tim - time, successCount, failCount, spaces);
            } else {
                successCount++;
                unsigned long tim = micros();
                radio.read(gotBuff, sizeof(gotBuff));
//                printf("Got response %s, delay: %lu microsec. sucs/fail: %d/%d\r\n",
//                        hexStr(gotBuff, sizeof(gotBuff), strBuf), tim - time, successCount, failCount);
                inc(counter, sizeof(counter));
            }
        }

        // Try again later
        //delay(10);

        serialTime += micros() - time;
        if (serialTime > 3000000lu) {
            printf("sucs/fail: %d/%d, speed: %d bytes/sec, period: %lu\r\n", successCount, failCount, successCount * sizeof(counter) / 8, serialTime);
            successCount = 0;
            serialTime = 0;
            failCount = 0;
        }
    }

    // Pong back role.  Receive each packet, dump it out, and send it back

    if (role == role_pong_back) {
        byte pipeNo;                                    // Dump the payloads until we've gotten everything
        while (radio.available(&pipeNo)) {
            memset(gotBuff, 0, sizeof(gotBuff));
            radio.read(gotBuff, sizeof(gotBuff));
            radio.writeAckPayload(pipeNo, gotBuff, sizeof(gotBuff));
        }
    }

    // Change roles

    if (Serial.available()) {
        char c = toupper(Serial.read());
        if (c == 'T' && role == role_pong_back) {
            Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));

            role = role_ping_out;                  // Become the primary transmitter (ping out)
            radio.openWritingPipe(pipes[0]);
            radio.openReadingPipe(1, pipes[1]);
        } else if (c == 'R' && role == role_ping_out) {
            Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));

            role = role_pong_back;                // Become the primary receiver (pong back)
            radio.openWritingPipe(pipes[1]);
            radio.openReadingPipe(1, pipes[0]);
            radio.startListening();
        }
    }
}
