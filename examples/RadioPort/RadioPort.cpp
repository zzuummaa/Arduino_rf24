//
// Created by Stephan on 23.11.2018.
//

#include "RadioPort.h"

const static byte ACK_PAYLOAD[] = {0xFF};

RadioPort::RadioPort(): RF24(7, 8), timeout_mcs(10 * 1000lu), ack_timeout_msc(2 * 1000) {}

void RadioPort::begin(role_e role) {
    RF24::begin();
    setAutoAck(true);
    enableAckPayload();               // Allow optional ack payloads
    setRetries(5, 15);                 // Smallest time between retries, max no. of retries
    enableDynamicPayloads();
    if (role == role_ping_out) {
        openWritingPipe(pipes[0]);
        openReadingPipe(1, pipes[1]);
    } else {
        openWritingPipe(pipes[1]);
        openReadingPipe(1, pipes[0]);
    }
    setDataRate(RF24_2MBPS);
    startListening();
}

int RadioPort::transmit(uint8_t *buff, int buffLen, uint8_t **notWriten) {
    unsigned long time = 0;
    unsigned long curTime;
    unsigned long ackTime = 0;
    int packetLen;
    int writenLen = 0;
    byte ackPayload;
    bool isEnd = false;
    bool status;
    bool isNeedACK = false;

    flush_tx();

    state = WRITE_PACKET;
    do {
        curTime = micros();

        switch (state) {
            case WRITE_PACKET:
                packetLen = writenLen + 32 <= buffLen ? 32 : buffLen - writenLen;

                stopListening();

                if (!write(buff, (uint8_t)packetLen)) {
                    printf("no write %d bytes\n\r", packetLen);
                    break;
                }

                state = READ_ACK;
                break;
            case READ_ACK:
                ackTime += micros() - curTime;

                if (ackTime > ack_timeout_msc) {
                    ackTime = 0;
                    state = WRITE_PACKET;
                    printf("READ_ACK timeout\n\r");
                    isEnd = true;
                }

                if (!available()) break;

                read(&ackPayload, sizeof(ackPayload));
                printf("packLen: %d, writenLen: %d\n\r", packetLen, writenLen);
                buff += packetLen;
                writenLen += packetLen;
                if (writenLen == buffLen) isEnd = true;

                break;
            default: printf("transmit invalid state %d\n\r", state);
        }

        time += micros() - curTime;
    } while (time < timeout_mcs && !isEnd);

    if (buffLen == writenLen && notWriten != nullptr) {
        *notWriten = buff;
    }

    return writenLen;
}

void RadioPort::setTimeout(unsigned long timeout) {
    this->timeout_mcs = timeout * 1000;
}

int RadioPort::receive(uint8_t *buff, int buffLen, unsigned long timeout) {
    if (timeout == 0) timeout = timeout_mcs;
    unsigned long time = 0;
    unsigned long curTime;
    int maxPacketLen;
    int readLen;
    int usedLen = 0;

    flush_rx();
    startListening();

    do {
        curTime = micros();

        maxPacketLen = usedLen + 32 <= buffLen ? 32 : buffLen - usedLen;
        readLen = readPacket(buff, maxPacketLen);
        printf("packLen: %d, usedLen: %d, readLen: %d\n\r", maxPacketLen, usedLen, readLen);
//        delay(100);
        if (readLen != 0) {
            buff += readLen;
            usedLen += readLen;
            if (buffLen == usedLen) break;
        }

        time += micros() - curTime;
    } while (time < timeout);

    stopListening();

    return usedLen;
}

bool RadioPort::writePacket(uint8_t* buff, int buffLen) {
    stopListening();

    if (!write(buff, (uint8_t)buffLen)) {
//        printf("no write %d bytes\n", buffLen);
        return false;
    }

    delayMicroseconds(10);
    if (!available()) {
        printf("no ack\n\r");
        return false;
    }

    byte ackPayload;
    while (available()) {
        read(&ackPayload, sizeof(ackPayload));
    }

    return ackPayload == 0xFF;
}

int RadioPort::readPacket(uint8_t *buff, int buffLen) {
    byte pipeNo;
    if (!available(&pipeNo)) {
        return 0;
    }

    uint8_t len = getDynamicPayloadSize();

    if (len == 0 | buffLen < len) {
        printf("len: %d, buffLen: %d\n\r", len, buffLen);
        return 0;
    }

    read(buff, len);
    writeAckPayload(pipeNo, &ACK_PAYLOAD, sizeof(ACK_PAYLOAD));

    return len;
}

unsigned long RadioPort::getTimeout() const {
    return timeout_mcs / 1000;
}
