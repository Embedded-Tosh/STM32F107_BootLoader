#ifndef CRC_H
#define CRC_H

#include <stdint.h>

/* CRC16 (Modbus, poly 0xA001, init 0xFFFF) - frame check, matches flash_tool.py.
 * Feed one byte at a time, carrying the running value between calls. */
uint16_t Crc16_Update(uint16_t crc, uint8_t byte);

/* Standard CRC-32 (zlib/PNG/Ethernet): poly 0xEDB88320 reflected, init/final
 * 0xFFFFFFFF. Bit-by-bit, no lookup table, to keep the flash footprint small.
 * patch_fw_metadata.ps1 computes this exact same algorithm on the PC. */
uint32_t Crc32_Compute(const uint8_t *data, uint32_t len);

/* Same CRC-32 in pieces, for data that is not one contiguous block (the
 * firmware image is checked as "before the footer" + "after the footer"):
 *     crc = Crc32_Init();
 *     crc = Crc32_Update(crc, part1, len1);
 *     crc = Crc32_Update(crc, part2, len2);
 *     result = Crc32_Final(crc);                                        */
uint32_t Crc32_Init(void);
uint32_t Crc32_Update(uint32_t crc, const uint8_t *data, uint32_t len);
uint32_t Crc32_Final(uint32_t crc);

#endif

