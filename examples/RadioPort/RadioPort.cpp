//
// Created by Stephan on 23.11.2018.
//

#include "RadioPort.h"

const static byte ACK_PAYLOAD[] = {0xFF};
bool flag = false;

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
    int maxPacketLen;
    byte ackPayload;
    bool isEnd = false;
    bool status;
    bool isNeedACK = false;
    counter = 1;

    stopListening();
    flush_rx();
    flush_tx();

    state = WRITE_PACKET;
    do {
        curTime = micros();

        maxPacketLen = writenLen + 31 <= buffLen ? 31 : buffLen - writenLen;
        packetLen = writePacket(buff, maxPacketLen);
        if (packetLen == -2) break;
        if (packetLen != -1) {
            buff += packetLen;
            writenLen += packetLen;
            printf("maxPackLen: %d, currPackLen: %d, writtenBytes: %d, counter: %d\n\r", maxPacketLen, packetLen, writenLen, counter);
            if (writenLen == buffLen) break;
        }

        time += micros() - curTime;
    } while (time < timeout_mcs);

    if (buffLen == writenLen && notWriten != nullptr) {
        printf("writing end packets...\n\r");
        do { counter = 0; } while (writePacket(buff, 0) != -2);
        *notWriten = buff;
    }

    return writenLen;
}

void RadioPort::setTimeout(unsigned long timeout) {
    this->timeout_mcs = timeout * 1000lu;
}

int RadioPort::receive(uint8_t *buff, int buffLen, unsigned long timeout) {
    if (timeout == 0) timeout = timeout_mcs;
    unsigned long time = 0;
    unsigned long curTime;
    uint8_t maxPacketLen;
    int readLen;
    int usedLen = 0;
    counter = 1;

    flush_rx();
    flush_tx();
    startListening();

    curTime = micros();
    do {

        maxPacketLen = (uint8_t)(usedLen + 31 <= buffLen ? 31 : buffLen - usedLen);
        readLen = readPacket(buff, maxPacketLen);
        if (readLen == -2) break;
        if (readLen != -1) {
            buff += readLen;
            usedLen += readLen;
            //printf("maxPackLen: %d, curPackLen: %d, readBytes: %d, counter: %d, time: %lu\n\r", maxPacketLen, readLen, usedLen, counter, micros() - curTime);
            if (buffLen == usedLen) break;
        }

    } while (micros() - curTime < timeout);

    if (readLen == -2) {
        printf("reading end packet...\n\r");
        do { counter = 0; } while (readPacket(buff, 0) != -1);
    }

    return usedLen;
}

int RadioPort::writePacket(uint8_t* buff, int buffLen) {
    uint8_t outBuff[buffLen+1];
    outBuff[0] = counter;
    memcpy(outBuff+1, buff, sizeof(outBuff)-1);

    stopListening();
    if (!write(&outBuff, sizeof(outBuff))) {
        printf("err write\n\r");
        return -1;
    }

    if (!isAckPayloadAvailable()) {
        printf("err no ack\n\r");
        return -1;
    }

    uint8_t ackPayload;
    read(&ackPayload, 1);
    if (ackPayload == 0) {
        printf("End ack packet received\n\r");
        return -2;
    }
    if (ackPayload != counter) {
        printf("err counter: %d, ackPayload: %d\n\r", counter, ackPayload);
        return -1;
    }

    counter++;
    return buffLen;
}

int RadioPort::readPacket(uint8_t *buff, uint8_t buffLen) {
    if (flag) {
        printf("ReadPacket 200\n\r");
        delay(30);
    }

    byte pipeNo;
    if (!available(&pipeNo)) {
        return -1;
    }

    uint8_t len = getDynamicPayloadSize();

    if (len == 0) {
        printf("err payloadLen: %d, buffLen: %d\n\r", len, buffLen);
        return -1;
    }

    uint8_t inBuff[buffLen+1];
    read(inBuff, len);
    writeAckPayload(pipeNo, &counter, sizeof(counter));
    if (len == 1) {
        printf("End packet received\n\r");
        return -2;
    }

    if (inBuff[0] != counter) {
        printf("err counter: %d, payloadCounter: %d\n\r", counter, inBuff[0]);
        if (buffLen != 31) flag = true;
        return -1;
    }

    counter++;
    if (buffLen >= sizeof(inBuff)-1) {
        memcpy(buff, inBuff+1, sizeof(inBuff)-1);
        return sizeof(inBuff)-1;
    } else {
        memcpy(buff, inBuff+1, (size_t)buffLen);
        printf("err buffLen: %d, payloadLen: %d\n\r", buffLen, len);
        return buffLen;
    }
}

unsigned long RadioPort::getTimeout() const {
    return timeout_mcs / 1000;
}
