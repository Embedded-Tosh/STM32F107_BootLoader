#ifndef FW_VALIDATE_H
#define FW_VALIDATE_H

#include <stdint.h>

/* Lightweight check used only by CMD_CHECK, against the NEW image's header as
 * sent by the host (not yet written to flash). Only magic + machine_id are
 * meaningful here - image_size/crc32 describe bytes that don't exist on this
 * device yet. h must point at FW_HEADER_SIZE (20) bytes. */
uint8_t Header_IsValid_ForCheck(const uint8_t *h);

/* Application in flash is plausible (stack pointer / reset vector) AND its
 * footer has the right magic, this machine's ID, a sane image_size, and a
 * CRC32 that matches the actually-flashed bytes. Run at every boot and before
 * every jump - a bad or corrupted image never runs. */
uint8_t App_IsValid(void);

#endif
