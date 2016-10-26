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

class HDLC_TL1B:
        private HDLC
{
public:
    static const uint16_t RXBFLEN = HDLC::RXBFLEN - 1U;

    HDLC_TL1B();
    void init();

    void transmitReset();

    void transmitBlock(const void* vdata, uint16_t len);

    void transmitStart();
    void transmitByte(uint8_t data);
    void transmitBytes(const void* vdata, uint16_t len);
    void transmitEnd();

    uint16_t receive();
    uint16_t copyReceivedMessage(uint8_t (&buff)[RXBFLEN]);

private:
    void transmitAck(uint8_t rxs);
    void transmitNack(uint8_t rxs);

    uint8_t count_seq;
    uint8_t count_tx_noack;
};

#endif /* HDLC_TL1B_H_ */
