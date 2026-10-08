#ifndef FLASH_IF_H
#define FLASH_IF_H

#include <stdint.h>

/* Erases the whole application region (APP_ADDRESS .. +APP_MAX_SIZE) and
 * rewinds the write pointer. Returns 1 on success, 0 on failure. */
uint8_t  Flash_EraseAppRegion(void);

/* Writes len bytes (must be a multiple of 4; caller pads with 0xFF) at the
 * current write pointer, verifying each word by read-back, then advances the
 * pointer. Returns 1 on success, 0 on failure or if it would overflow the
 * application region. */
uint8_t  Flash_WriteChunk(const uint8_t *data, uint16_t len);

void     Flash_ResetWritePtr(void);      /* back to APP_ADDRESS */
uint32_t Flash_GetWritePtr(void);        /* next address to be written */

#endif
