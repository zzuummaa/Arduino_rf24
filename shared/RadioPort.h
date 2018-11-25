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

    size_t transmit(uint8_t *buff, size_t buffLen);

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
    unsigned long curTime;
    uint8_t counter;
    unsigned long ack_timeout_msc;
    unsigned long timeout_mcs;
    uint64_t pipes[2] = {0xABCDABCD71LL,
                         0x544d52687CLL};              // Radio pipe addresses for the 2 nodes to communicate.
};

inline int printf_moc(const char *__fmt, ...);

enum LogType {
    NON = 0,
    INFO,
    DEBUG
};

#define RADIO_PORT_LOG_TYPE LogType::NON

#endif //ARDUINO_RF24_RADIOPORT_H
