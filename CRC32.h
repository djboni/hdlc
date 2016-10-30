#ifndef CRC32_H_
#define CRC32_H_

#include <stdint.h>
#include <avr/pgmspace.h>

struct CRC32 {
    typedef uint32_t CRC_t;
    static const int8_t size        = 4;
    static const CRC_t CRC_INIT     = 0xFFFFFFFFUL;
    static const CRC_t CRC_FINALXOR = 0xFFFFFFFFUL;
    static const CRC_t CRC_GOOD     = 0xDEBB20E3UL;

    PROGMEM static const CRC_t CRC_TAB[256U];

    void init() { crc = CRC_INIT; }
    void update(uint8_t data) {
        /* crc = (crc >> 8U) ^ CRC_TAB[(crc ^ data) & 0xFFU]; */
        crc = (crc >> 8U) ^ pgm_read_dword(&CRC_TAB[(crc ^ data) & 0xFFU]);
    }

    bool good() { return crc == CRC_GOOD; }

    void final() { crc ^= CRC_FINALXOR; }
    uint8_t operator[](int8_t pos) {
        switch(pos) {
            case 0:  return crc;
            case 1:  return crc >> 8U;
            case 2:  return crc >> 16U;
            default: return crc >> 24U;
        }
    }

    CRC_t crc;
};

#endif /* CRC32_H_ */
