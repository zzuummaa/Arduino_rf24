//
// Created by Stephan on 23.11.2018.
//

#include "RadioPort.h"

#define printf_L(log, fmt, args...) if (log <= RADIO_PORT_LOG_TYPE) printf(fmt, ## args)

RadioPort::RadioPort(): RF24(7, 8), timeout_mcs(20 * 1000lu), ack_timeout_msc(2 * 1000) {}

void RadioPort::begin(Role role) {
    RF24::begin();
    setAutoAck(true);
    enableAckPayload();               // Allow optional ack payloads
    setRetries(5, 15);                 // Smallest time between retries, max no. of retries
    enableDynamicPayloads();
    if (role == TRANSMITTER) {
        openWritingPipe(pipes[0]);
        openReadingPipe(1, pipes[1]);
    } else {
        openWritingPipe(pipes[1]);
        openReadingPipe(1, pipes[0]);
    }
    setDataRate(RF24_2MBPS);
}

size_t RadioPort::transmit(uint8_t *buff, size_t buffLen) {
    unsigned long timeout = timeout_mcs;
    int packetLen;
    size_t writenLen = 0;
    int maxPacketLen;
    counter = 1;

    stopListening();
    flush_rx();
    flush_tx();

    curTime = micros();
    do {
        maxPacketLen = writenLen + 31 <= buffLen ? 31 : buffLen - writenLen;
        packetLen = writePacket(buff, maxPacketLen);
        if (packetLen == -2) break;
        if (packetLen > 0) {
            buff += packetLen;
            writenLen += packetLen;
            printf_L(INFO, "maxPackLen: %d, currPackLen: %d, writtenBytes: %d, counter: %d\n\r", maxPacketLen, packetLen, writenLen, counter);
            curTime = micros();
            if (writenLen == buffLen) break;
        }
    } while (micros() - curTime < timeout);

    if (buffLen == writenLen && micros() - curTime < timeout) {
        printf_L(INFO, "writing end packet...\n\r");
        counter = 0;
        packetLen = writePacket(buff, 0);
        if (packetLen > 0) {
            printf_L(INFO, "maxPackLen: %d, currPackLen: %d, writtenBytes: %d, counter: %d\n\r", 0, packetLen, writenLen, counter);
        }
    }

    return writenLen;
}

void RadioPort::setTimeout(unsigned long timeout) {
    this->timeout_mcs = timeout * 1000lu;
}

size_t RadioPort::receive(uint8_t *buff, size_t buffLen, unsigned long timeout) {
    if (timeout == 0) timeout = timeout_mcs;
    uint8_t maxPacketLen;
    int readLen = 0;
    size_t usedLen = 0;
    counter = 1;

    flush_rx();
    flush_tx();
    startListening();

    curTime = micros();
    do {

        maxPacketLen = (uint8_t)(usedLen + 31 <= buffLen ? 31 : buffLen - usedLen);
        readLen = readPacket(buff, maxPacketLen);
        if (readLen == -2) break;
        if (readLen > 0) {
            buff += readLen;
            usedLen += readLen;
//            printf_L("maxPackLen: %d, curPackLen: %d, readBytes: %d, counter: %d, time: %lu\n\r", maxPacketLen, readLen, usedLen, counter, micros() - curTime); // Reset
            printf_L(INFO, "readNow: %d, readAllBytes: %d, counter: %d, time: %lu\n\r", readLen, usedLen, counter, micros() - curTime); // Not reset
//            printf_L("maxPackLen: %d, curPackLen: %d\n\r", maxPacketLen, readLen); // Reset
            curTime = micros();
            if (buffLen == usedLen) break;
        }

    } while (micros() - curTime < timeout);

    stopListening();

    return usedLen;
}

int RadioPort::writePacket(uint8_t* buff, int buffLen) {
    uint8_t outBuff[buffLen+1];
    outBuff[0] = counter;
    memcpy(outBuff+1, buff, sizeof(outBuff)-1);

    stopListening();
    if (!write(&outBuff, sizeof(outBuff))) {
        printf_L(DEBUG, "err write\n\r");
        return -1;
    }

    uint8_t ackPayload;
    while (isAckPayloadAvailable()) {
        read(&ackPayload, 1);
        if (ackPayload == 0) {
            printf_L(DEBUG, "End ack packet received\n\r");
            return -2;
        }
        if (ackPayload != counter) {
            printf_L(DEBUG, "err counter: %d, ackPayload: %d\n\r", counter, ackPayload);
        } else {
            counter++;
            return buffLen;
        }
    }

    printf_L(DEBUG, "err no ack\n\r");
    return -1;
}

int RadioPort::readPacket(uint8_t *buff, uint8_t buffLen) {
    byte pipeNo;
    if (!available(&pipeNo)) {
        return -1;
    }

    uint8_t len = getDynamicPayloadSize();

    if (len == 0) {
        printf_L(DEBUG, "err payloadLen: %d, buffLen: %d\n\r", len, buffLen);
        return -1;
    }

    uint8_t inBuff[len];
    read(inBuff, len);
    writeAckPayload(pipeNo, &counter, sizeof(counter));
    if (len == 1) {
        printf_L(DEBUG, "End packet received\n\r");
        return -2;
    }

    if (inBuff[0] != counter) {
        printf_L(DEBUG, "err counter: %d, payloadCounter: %d\n\r", counter, inBuff[0]);
        return -1;
    }

    counter++;
    if (buffLen >= sizeof(inBuff)-1) {
        memcpy(buff, inBuff+1, sizeof(inBuff)-1);
        return sizeof(inBuff)-1;
    } else {
        memcpy(buff, inBuff+1, (size_t)buffLen);
        printf_L(DEBUG, "err buffLen: %d, payloadLen: %d\n\r", buffLen, len);
        return buffLen;
    }
}

unsigned long RadioPort::getTimeout() const {
    return timeout_mcs / 1000;
}

int printf_moc(const char *__fmt, ...) {
    return 0;
}
