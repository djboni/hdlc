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

#include "HDLC.h"
#include <string.h>

#include <util/crc16.h>
#define CRC_INIT              0xFFFFU
#define CRC_FINALXOR          0xFFFFU
#define CRC_GOOD              0xF0B8U
#define crc_update(crc, data) _crc_ccitt_update(crc, data)

#include "Arduinutil.h"

static int16_t readByte(void)
{
    int16_t data = Serial1_read();
    return data;
}

static void writeByte(uint8_t data)
{
    Serial1_writeByte(data);
}

static void escapeAndWriteByte(uint8_t data)
{
    if(     data == '~' ||
            data == '}' ||
            data == '\n')
    {
        writeByte('}');
        writeByte(data ^ 0x20U);
    }
    else
    {
        writeByte(data);
    }
}

HDLC::HDLC()
{
    init();
}

void HDLC::init()
{
    len = 0U;
    status = RECEIVING;
    crc = CRC_INIT;
}

void HDLC::transmitBlock(const void* vdata, uint16_t len)
{
    const uint8_t* data = (const uint8_t*)vdata;
    transmitStart();
    while(len)
    {
        transmitByte(*data);
        ++data;
        --len;
    }
    transmitEnd();
}

void HDLC::transmitStart()
{
    writeByte('~');
    txcrc = CRC_INIT;
}

void HDLC::transmitByte(uint8_t data)
{
    escapeAndWriteByte(data);
    txcrc = crc_update(txcrc, data);
}

void HDLC::transmitEnd()
{
    txcrc ^= CRC_FINALXOR;
    escapeAndWriteByte(txcrc & 0xFFU);
    escapeAndWriteByte((txcrc >> 8U) & 0xFFU);

    writeByte('~');
}

uint16_t HDLC::receive()
{
    int16_t c = readByte();
    if(c == -1)
        return 0U;

    if(status >= OK)
        init();

    uint16_t retv = 0U;

    if(c == '~')
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

            c ^= 0x20U;
            crc = crc_update(crc, c);
            if(len < RXBFLEN)
                data[len] = c;
            ++len;
        }
        else if(c != '}')
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

uint16_t HDLC::copyReceivedMessage(uint8_t (&buff)[RXBFLEN])
{
    memcpy(buff, data, len);

    const uint16_t datalen = len;
    init();
    return datalen;
}

uint16_t HDLC::copyReceivedMessage(uint8_t *buff, uint16_t pos, uint16_t num,
        bool callinit)
{
    num = (pos + num) > len ? (len - pos) : num;

    memcpy(buff, &data[pos], num);

    if(callinit)
        init();

    return num;
}
