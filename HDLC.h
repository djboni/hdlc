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
public:
    static const uint16_t RXBFLEN = rxBuffLen;

    HDLC() {
        init();
    }

    void init() {
        len = 0U;
        status = RECEIVING;
        crc = CRC_INIT;
    }

    void transmitBlock(const void* vdata, uint16_t len) {
        transmitStart();
        transmitBytes(vdata, len);
        transmitEnd();
    }

    void transmitStart() {
        writeByte('~');
        txcrc = CRC_INIT;
    }

    void transmitByte(uint8_t data) {
        escapeAndWriteByte(data);
        txcrc = crc_update(txcrc, data);
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
        txcrc ^= CRC_FINALXOR;
        escapeAndWriteByte(txcrc & 0xFFU);
        escapeAndWriteByte((txcrc >> 8U) & 0xFFU);

        writeByte('~');
    }

    uint16_t receive() {
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

    uint16_t copyReceivedMessage(uint8_t (&buff)[RXBFLEN]) {
        const uint16_t datalen = (len > RXBFLEN) ? RXBFLEN : len;
        memcpy(buff, data, datalen);
        init();
        return datalen;
    }

    uint16_t copyReceivedMessage(uint8_t *buff, uint16_t pos, uint16_t num,
            bool callinit=false) {
        const uint16_t datalen = (len > RXBFLEN) ? RXBFLEN : len;
        num = (pos + num) > datalen ? (datalen - pos) : num;
        memcpy(buff, &data[pos], num);
        if(callinit)
            init();
        return num;
    }

private:

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

#endif /* HDLC_H_ */
