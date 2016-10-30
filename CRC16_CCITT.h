#ifndef CRC16_CCITT_H_
#define CRC16_CCITT_H_

#include <stdint.h>
#include <util/crc16.h>

struct CRC16_CCITT {
    typedef uint16_t CRC_t;
    static const int8_t size        = 2;
    static const CRC_t CRC_INIT     = 0xFFFFU;
    static const CRC_t CRC_FINALXOR = 0xFFFFU;
    static const CRC_t CRC_GOOD     = 0xF0B8U;

    void init() { crc = CRC_INIT; }
    void update(uint8_t data) { crc = _crc_ccitt_update(crc, data); }

    bool good() { return crc == CRC_GOOD; }

    void final() { crc ^= CRC_FINALXOR; }
    uint8_t operator[](int8_t pos) {
        switch(pos) {
            case 0:  return crc;
            default: return crc >> 8U;
        }
    }

    CRC_t crc;
};

#endif /* CRC16_CCITT_H_ */
