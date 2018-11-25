//
// Created by Stephan on 23.11.2018.
//

#ifndef ARDUINO_RF24_RADIOPORT_H
#define ARDUINO_RF24_RADIOPORT_H

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

enum Role {
    TRANSMITTER = 1, RECEIVER
};                 // The various roles supported by this sketch

class RadioPort: public RF24 {
public:
    enum State {
        WRITE_PACKET,
        READ_ACK,
        READ_PACKET,
        WRITE_ACK
    };


    RadioPort();

    void begin(Role role);

    size_t transmit(uint8_t *buff, size_t buffLen, uint8_t **notWriten = nullptr);

    size_t receive(uint8_t *buff, size_t buffLen, unsigned long timeout = 0);

    int writePacket(uint8_t* buff, int buffLen);

    int readPacket(uint8_t* buff, uint8_t buffLen);

    void setTimeout(unsigned long timeout);

    unsigned long getTimeout() const;

    void printDetails() {
        printf("timeout\t\t = %lu ms\n\r", timeout_mcs / 1000);
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

inline int printf_moc(const char *__fmt, ...);

#define RADIO_PORT_NON 0
#define RADIO_PORT_INFO 1
#define RADIO_PORT_DEBUG 2

#define RADIO_PORT_LOG_TYPE RADIO_PORT_INFO

#if RADIO_PORT_LOG_TYPE > RADIO_PORT_NON
    #define printf_I(fmt, args...) printf(fmt, ## args)
    #if RADIO_PORT_LOG_TYPE > RADIO_PORT_INFO
        #define printf_D(fmt, args...) printf(fmt, ## args)
    #else
        #define printf_D(fmt, args...) printf_moc(fmt, ## args)
    #endif
#else
    #define printf_I(fmt, args...) printf_moc(fmt, ## args)
    #define printf_D(fmt, args...) printf_moc(fmt, ## args)
#endif

#endif //ARDUINO_RF24_RADIOPORT_H
