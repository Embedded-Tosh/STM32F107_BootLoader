/*******************************************************************************
 * bootloader.c
 *
 * UART4 + UART5 bootloader for STM32F107VCT6 (Standard Peripheral Library)
 *
 * Memory map assumed:
 *   Bootloader : 0x08000000 - 0x08002FFF   (12 KB)
 *   Application: 0x08003000 - 0x0803FFFF   (244 KB)
 *
 * UART4 pins (fixed, no remap on F107): PC10 = TX, PC11 = RX
 * UART5 pins (fixed, no remap on F107): PC12 = TX, PD2  = RX
 * Both ports are listened to at the same time. Each frame is answered on
 * the port it arrived on, so the host can use either one.
 *
 * Protocol (host -> boot):
 *   [0xAA][CMD][LEN_LO][LEN_HI][...payload...][CRC16_LO][CRC16_HI]
 *   CRC16 (Modbus poly 0xA001) computed over CMD+LEN_LO+LEN_HI+payload
 *
 * Boot -> host: single byte reply
 *   0x06 = ACK      0x15 = NACK
 *
 * Commands:
 *   CMD_SYNC   0x01  - no payload.                     -> ACK if bootloader alive
 *   CMD_CHECK  0x05  - payload = 20-byte firmware ID header.  -> ACK only if password + machine ID match.
 *                     Required before CMD_ERASE (a wrong image never erases the old one). The host
 *                     reads these 20 bytes from the fixed FW_METADATA address (see fw_metadata.c
 *                     and the application's scatter file, app_scatter.sct). image_size/crc32 in this
 *                     payload describe the NEW image and aren't checked here - they can't be, since
 *                     nothing has been written yet. The real integrity check happens after the write,
 *                     reading image_size/crc32 back from flash - see Find_And_ValidateHeader().
 *   CMD_ERASE  0x02  - no payload.                      -> ACK after full app region erased
 *   CMD_WRITE  0x03  - payload = up to 1024 bytes.       -> ACK per chunk, written sequentially
 *   CMD_GO     0x04  - no payload.                       -> ACK, then jumps to application, but only
 *                     if Find_And_ValidateHeader() confirms magic + machine ID + CRC32 against the
 *                     actual flashed bytes (same check runs on every normal boot too - a bad or
 *                     corrupted image never runs)
 *
 * Build this as its OWN Keil project with its own scatter file:
 *   IROM1 base = 0x08000000, size = 0x00003000
 *
 * Author: Aashutosh
 * Linkedin: aashutosh-535016185 
 ******************************************************************************/
