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

class HDLC
{
public:
    static const uint16_t RXBFLEN = 129U;

    HDLC();
    void transmit(const void* vdata, uint16_t len) const;
    uint16_t receive();
    void copyReceivedMessage(uint8_t rxbf[RXBFLEN]);

private:
    uint8_t stat;
    uint16_t len;
    uint16_t crc;
    uint8_t data[RXBFLEN];
};

#endif /* HDLC_H_ */
