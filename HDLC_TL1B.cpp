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

#include "HDLC_TL1B.h"
#include <string.h>

#include "Arduinutil.h"

#define MASK    0xC0U
#define MASKINV 0x3FU
#define RESET   0x00U
#define ACK     0x40U
#define NACK    0x80U
#define DATA    0xC0U

#ifndef HDLC_SEQ_MAX
#define HDLC_SEQ_MAX    63U
#endif

#ifndef HDLC_NOACK_LIM
#define HDLC_NOACK_LIM  5U
#endif

HDLC_TL1B::HDLC_TL1B()
{
    init();
}

void HDLC_TL1B::init()
{
    HDLC::init();
    count_seq = HDLC_SEQ_MAX;
    count_tx_noack = 0U;
}

void HDLC_TL1B::transmitReset()
{
    init();
    HDLC::transmitStart();
    HDLC::transmitByte(RESET);
    HDLC::transmitEnd();
}

void HDLC_TL1B::transmitAck(uint8_t rxs)
{
    rxs &= MASKINV;
    rxs |= ACK;
    HDLC::transmitStart();
    HDLC::transmitByte(rxs);
    HDLC::transmitEnd();
}

void HDLC_TL1B::transmitNack(uint8_t rxs)
{
    rxs &= MASKINV;
    rxs |= NACK;
    HDLC::transmitStart();
    HDLC::transmitByte(rxs);
    HDLC::transmitEnd();
}

void HDLC_TL1B::transmitBlock(const void* vdata, uint16_t len)
{
    transmitStart();
    transmitBytes(vdata, len);
    transmitEnd();
}

void HDLC_TL1B::transmitStart()
{
    if(++count_tx_noack >= HDLC_NOACK_LIM)
        transmitReset();

    HDLC::transmitStart();
    count_seq = (count_seq < HDLC_SEQ_MAX) ? (count_seq + 1U) : 0U;
    HDLC::transmitByte(DATA | count_seq);
}

void HDLC_TL1B::transmitByte(uint8_t data)
{
    HDLC::transmitByte(data);
}

void HDLC_TL1B::transmitBytes(const void* vdata, uint16_t len)
{
    const uint8_t* data = (const uint8_t*)vdata;
    while(len)
    {
        transmitByte(*data);
        ++data;
        --len;
    }
}

void HDLC_TL1B::transmitEnd()
{
    HDLC::transmitEnd();
}

uint16_t HDLC_TL1B::receive()
{
    uint16_t datalen = HDLC::receive();
    if(datalen != 0U)
    {
        datalen -= 1U;

        uint8_t frameseq;
        HDLC::copyReceivedMessage(&frameseq, 0U, 1U);

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

uint16_t HDLC_TL1B::copyReceivedMessage(uint8_t (&buff)[RXBFLEN])
{
    uint16_t datalen = HDLC::copyReceivedMessage(buff, 1U, RXBFLEN, true);
    return datalen;
}
