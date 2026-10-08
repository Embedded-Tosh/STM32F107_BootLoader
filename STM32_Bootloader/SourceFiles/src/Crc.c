/* Crc.c - see Crc.h */
#include "Crc.h"

uint16_t Crc16_Update(uint16_t crc, uint8_t byte)
{
    uint8_t j;
    crc ^= byte;
    for (j = 0; j < 8; j++)
    {
        if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
        else              crc = (crc >> 1);
    }
    return crc;
}

uint32_t Crc32_Init(void)
{
    return 0xFFFFFFFFUL;
}

uint32_t Crc32_Update(uint32_t crc, const uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint8_t  j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320UL) : (crc >> 1);
        }
    }
    return crc;
}

uint32_t Crc32_Final(uint32_t crc)
{
    return crc ^ 0xFFFFFFFFUL;
}

uint32_t Crc32_Compute(const uint8_t *data, uint32_t len)
{
    return Crc32_Final(Crc32_Update(Crc32_Init(), data, len));
}



