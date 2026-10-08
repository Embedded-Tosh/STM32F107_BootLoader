/*******************************************************************************
 * Protocol.h - host <-> bootloader framing
 *
 * Host -> boot:  [0xAA][CMD][LEN_LO][LEN_HI][...payload...][CRC16_LO][CRC16_HI]
 *                CRC16 (Modbus, poly 0xA001) over CMD+LEN_LO+LEN_HI+payload
 * Boot -> host:  single byte, 0x06 = ACK, 0x15 = NACK
 *
 * Commands:
 *   CMD_SYNC   no payload             -> ACK if the bootloader is alive
 *   CMD_CHECK  20-byte firmware ID    -> ACK only if magic + machine ID match.
 *              Required before CMD_ERASE, so a wrong image never erases the old
 *              one. image_size/crc32 in this payload describe the NEW image and
 *              can't be checked yet; the real integrity check runs after the
 *              write, from flash (see FwValidate.c).
 *   CMD_ERASE  no payload             -> ACK after the full app region is erased
 *   CMD_WRITE  up to MAX_PAYLOAD bytes-> ACK per chunk, written sequentially
 *   CMD_GO     no payload             -> ACK, then jumps to the application, but
 *              only if App_IsValid() passes (same check runs on every boot)
 ******************************************************************************/
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define FRAME_START_BYTE    0xAA
#define ACK_BYTE            0x06
#define NACK_BYTE           0x15

#define CMD_SYNC            0x01
#define CMD_ERASE           0x02
#define CMD_WRITE           0x03
#define CMD_GO              0x04
#define CMD_CHECK           0x05

/* Receives one frame on the active UART. payload must hold MAX_PAYLOAD bytes.
 * Returns 1 only for a complete frame with a valid CRC16. */
uint8_t ReceiveFrame(uint8_t *cmd, uint8_t *payload, uint16_t *payload_len,
                     uint32_t byte_timeout_ms);

#endif
