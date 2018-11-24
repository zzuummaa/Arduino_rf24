//
// Created by Stephan on 23.11.2018.
//

#ifndef ARDUINO_RF24_RADIOPORT_H
#define ARDUINO_RF24_RADIOPORT_H


#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

typedef enum {
    role_ping_out = 1, role_pong_back
} role_e;                 // The various roles supported by this sketch

class RadioPort: public RF24 {
public:
    enum State {
        WRITE_PACKET,
        READ_ACK,
        READ_PACKET,
        WRITE_ACK
    };


    RadioPort();

    void begin(role_e role);

    int transmit(uint8_t* buff, int buffLen, uint8_t **notWriten = nullptr);

    int receive(uint8_t* buff, int buffLen, unsigned long timeout = 0);

    int writePacket(uint8_t* buff, int buffLen);

    int readPacket(uint8_t* buff, int buffLen);

    void setTimeout(unsigned long timeout);

    unsigned long getTimeout() const;

    void printDetails() {
        RF24::printDetails();
    }
private:
    uint8_t counter;
    State state;
    unsigned long ack_timeout_msc;
    unsigned long timeout_mcs;
    uint64_t pipes[2] = {0xABCDABCD71LL,
                         0x544d52687CLL};              // Radio pipe addresses for the 2 nodes to communicate.
};


#endif //ARDUINO_RF24_RADIOPORT_H
