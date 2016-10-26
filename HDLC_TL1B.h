/*
 Copyright 2016 Djones A. Boni

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#ifndef HDLC_TL1B_H_
#define HDLC_TL1B_H_

#include "HDLC.h"

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen, uint8_t seqMax = 63U, uint8_t noAckLim = 5U>
class HDLC_TL1B:
        private HDLC<readByte, writeByte, rxBuffLen + 1U>
{
private:
    static const uint8_t MASK    = 0xC0U;
    static const uint8_t MASKINV = 0x3FU;
    static const uint8_t RESET   = 0x00U;
    static const uint8_t ACK     = 0x40U;
    static const uint8_t NACK    = 0x80U;
    static const uint8_t DATA    = 0xC0U;

public:
    static const uint16_t RXBFLEN = rxBuffLen;
    static const uint16_t BASE_RXBFLEN = rxBuffLen + 1U;

    HDLC_TL1B() {
        init();
    }

    void init() {
        HDLC<readByte, writeByte, BASE_RXBFLEN>::init();
        count_seq = seqMax;
        count_tx_noack = 0U;
    }

    void transmitReset() {
        init();
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitStart();
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitByte(RESET);
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitEnd();
    }

    void transmitBlock(const void* vdata, uint16_t len) {
        transmitStart();
        transmitBytes(vdata, len);
        transmitEnd();
    }

    void transmitStart() {
        if(++count_tx_noack >= noAckLim)
            transmitReset();

        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitStart();
        count_seq = (count_seq < seqMax) ? (count_seq + 1U) : 0U;
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitByte(DATA | count_seq);
    }

    void transmitByte(uint8_t data) {
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitByte(data);
    }

    void transmitBytes(const void* vdata, uint16_t len) {
        const uint8_t* data = (const uint8_t*)vdata;
        while(len)
        {
            transmitByte(*data);
            ++data;
            --len;
        }
    }

    void transmitEnd() {
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitEnd();
    }

    uint16_t receive() {
        uint16_t datalen = HDLC<readByte, writeByte, BASE_RXBFLEN>::receive();
        if(datalen != 0U)
        {
            datalen -= 1U;

            uint8_t frameseq;
            HDLC<readByte, writeByte, BASE_RXBFLEN>::copyReceivedMessage(&frameseq, 0U, 1U);

            uint8_t frame = frameseq & MASK;
            uint8_t rxs = frameseq & MASKINV;
            if(frame == DATA)
            {
                transmitAck(rxs);
            }
            else if(frame == ACK)
            {
                count_tx_noack = 0U;
            }
            else if(frame == NACK)
            {
            }
            else if(frame == RESET)
            {
            }
            else
            {
            }
        }
        return datalen;
    }

    uint16_t copyReceivedMessage(uint8_t (&buff)[RXBFLEN]) {
        uint16_t datalen = HDLC<readByte, writeByte, BASE_RXBFLEN>::copyReceivedMessage(buff, 1U, RXBFLEN, true);
        return datalen;
    }

private:
    void transmitAck(uint8_t rxs) {
        rxs &= MASKINV;
        rxs |= ACK;
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitStart();
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitByte(rxs);
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitEnd();
    }

    void transmitNack(uint8_t rxs) {
        rxs &= MASKINV;
        rxs |= NACK;
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitStart();
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitByte(rxs);
        HDLC<readByte, writeByte, BASE_RXBFLEN>::transmitEnd();
    }

    uint8_t count_seq;
    uint8_t count_tx_noack;
};

#endif /* HDLC_TL1B_H_ */
