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

#ifndef HDLC_H_
#define HDLC_H_

#include <stdint.h>
#include <string.h>

#include <util/crc16.h>
#define CRC_INIT              0xFFFFU
#define CRC_FINALXOR          0xFFFFU
#define CRC_GOOD              0xF0B8U
#define crc_update(crc, data) _crc_ccitt_update(crc, data)

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
class HDLC
{
private:
    static const uint8_t DATAINVBIT;
    static const uint8_t DATASTART;
    static const uint8_t DATAESCAPE;
    static const uint8_t DATAESCAPELIST[];

public:
    static const uint16_t RXBFLEN = rxBuffLen;

    HDLC();
    void init();

    void transmitBlock(const void* vdata, uint16_t len);

    void transmitStart();
    void transmitByte(uint8_t data);
    void transmitBytes(const void* vdata, uint16_t len);
    void transmitEnd();

    uint16_t receive();

    uint16_t copyReceivedMessage(uint8_t (&buff)[RXBFLEN]);
    uint16_t copyReceivedMessage(uint8_t *buff, uint16_t pos, uint16_t num,
            bool callinit=false);

private:
    static void escapeAndWriteByte(uint8_t data) {
        const uint8_t n = sizeof(DATAESCAPELIST)/sizeof(DATAESCAPELIST[0]);
        for(int8_t i = 0; i < n; ++i)
        {
            if(data == DATAESCAPELIST[i])
            {
                writeByte(DATAESCAPE);
                data ^= DATAINVBIT;
                break;
            }
        }
        writeByte(data);
    }

    enum {
        ESCAPED   = -1,
        RECEIVING = 0,
        OK        = 1,
        CRCERR    = 2
    };

    uint16_t txcrc;

    int8_t status;
    uint16_t len;
    uint16_t crc;
    uint8_t data[RXBFLEN];
};



template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
const uint8_t HDLC<readByte, writeByte, rxBuffLen>::
        DATAINVBIT = 0x20U;

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
const uint8_t HDLC<readByte, writeByte, rxBuffLen>::
        DATASTART  = '~';

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
const uint8_t HDLC<readByte, writeByte, rxBuffLen>::
        DATAESCAPE = '}';

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
const uint8_t HDLC<readByte, writeByte, rxBuffLen>::
        DATAESCAPELIST[] = { DATASTART, DATAESCAPE };



template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
HDLC<readByte, writeByte, rxBuffLen>::HDLC()
{
    init();
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
void HDLC<readByte, writeByte, rxBuffLen>::init()
{
    len = 0U;
    status = RECEIVING;
    crc = CRC_INIT;
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
void HDLC<readByte, writeByte, rxBuffLen>::
        transmitBlock(const void* vdata, uint16_t len)
{
    transmitStart();
    transmitBytes(vdata, len);
    transmitEnd();
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
void HDLC<readByte, writeByte, rxBuffLen>::transmitStart()
{
    writeByte(DATASTART);
    txcrc = CRC_INIT;
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
void HDLC<readByte, writeByte, rxBuffLen>::transmitByte(uint8_t data)
{
    escapeAndWriteByte(data);
    txcrc = crc_update(txcrc, data);
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
void HDLC<readByte, writeByte, rxBuffLen>::
        transmitBytes(const void* vdata, uint16_t len)
{
    const uint8_t* data = (const uint8_t*)vdata;
    while(len)
    {
        transmitByte(*data);
        ++data;
        --len;
    }
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
void HDLC<readByte, writeByte, rxBuffLen>::transmitEnd()
{
    txcrc ^= CRC_FINALXOR;
    escapeAndWriteByte(txcrc & 0xFFU);
    escapeAndWriteByte((txcrc >> 8U) & 0xFFU);

    writeByte(DATASTART);
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
uint16_t HDLC<readByte, writeByte, rxBuffLen>::receive()
{
    int16_t c = readByte();
    if(c == -1)
        return 0U;

    if(status >= OK)
        init();

    uint16_t retv = 0U;

    if(c == DATASTART)
    {
        if(status == RECEIVING && len != 0U)
        {
            if(crc == CRC_GOOD)
            {
                status = OK;
                len -= 2U;
                retv = len;
            }
            else
            {
                status = CRCERR;
            }
        }
        else
        {
            init();
        }
    }
    else
    {
        if(status == ESCAPED)
        {
            status = RECEIVING;

            c ^= DATAINVBIT;
            crc = crc_update(crc, c);
            if(len < RXBFLEN)
                data[len] = c;
            ++len;
        }
        else if(c != DATAESCAPE)
        {
            crc = crc_update(crc, c);
            if(len < RXBFLEN)
                data[len] = c;
            ++len;
        }
        else
        {
            status = ESCAPED;
        }
    }

    return retv;
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
uint16_t HDLC<readByte, writeByte, rxBuffLen>::
        copyReceivedMessage(uint8_t (&buff)[RXBFLEN])
{
    const uint16_t datalen = (len > RXBFLEN) ? RXBFLEN : len;
    memcpy(buff, data, datalen);
    init();
    return datalen;
}

template<int16_t (&readByte)(void), void (&writeByte)(uint8_t data),
        uint16_t rxBuffLen>
uint16_t HDLC<readByte, writeByte, rxBuffLen>::
        copyReceivedMessage(uint8_t *buff, uint16_t pos, uint16_t num,
                bool callinit)
{
    const uint16_t datalen = (len > RXBFLEN) ? RXBFLEN : len;
    num = (pos + num) > datalen ? (datalen - pos) : num;
    memcpy(buff, &data[pos], num);
    if(callinit)
        init();
    return num;
}

#endif /* HDLC_H_ */
