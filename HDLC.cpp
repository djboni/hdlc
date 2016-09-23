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
#include "Arduinutil.h"
#include <util/crc16.h>
#include <string.h>

static int16_t readByte(void)
{
    return Serial1_read();
}

static void writeByte(uint8_t data)
{
    Serial1_writeByte(data);
}

static void transmitByte(uint8_t data)
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

HDLC::HDLC():
    stat(0), len(0), crc(0)
{
}

void HDLC::transmit(const void* vdata, uint16_t len) const
{
    const uint8_t* data = (const uint8_t*)vdata;
    uint16_t crc = 0xFFFFU;

    writeByte('~');

    while(len--)
    {
        transmitByte(*data);
        crc = _crc_ccitt_update(crc, *data);
        ++data;
    }

    crc ^= 0xFFFFU;
    transmitByte(crc & 0xFFU);
    transmitByte((crc >> 8U) & 0xFFU);

    writeByte('~');
}
