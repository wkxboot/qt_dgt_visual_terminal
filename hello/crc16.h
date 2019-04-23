#ifndef CRC16_H
#define CRC16_H
#include "stdint.h"


class crc16
{
public:
    crc16();
    uint16_t calculate(uint8_t *src,int size);
};

#endif // CRC16_H
